/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#include "rtspclient/rtspclient.h"

#include "hobotlog/hobotlog.hpp"
#include "mediapipemanager/mediapipemanager.h"
#include "rtspclient/AudioG711Sink.h"
#include "rtspclient/H264Sink.h"
#include "rtspclient/H265Sink.h"
#include "rtspclient/rtspclient.h"

extern "C" {
#include "rtspclient/sps_pps.h"
}

// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode,
                           char *resultString);
void continueAfterSETUP(RTSPClient *rtspClient, int resultCode,
                        char *resultString);
void continueAfterPLAY(RTSPClient *rtspClient, int resultCode,
                       char *resultString);

// Other event handler functions:
void subsessionAfterPlaying(
    void *clientData);  // called when a stream's subsession (e.g., audio or
                        // video substream) ends
void subsessionByeHandler(
    void *clientData);  // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void *clientData);
// called at the end of a stream's expected duration (if the stream has not
// already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient *rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
// void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging
// output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env,
                             const RTSPClient &rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for
// debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env,
                             const MediaSubsession &subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
    : iter(NULL),
      session(NULL),
      subsession(NULL),
      streamTimerTask(NULL),
      duration(0.0) {}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if
    // set)
    UsageEnvironment &env = session->envir();  // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}

// Implementation of "ourRTSPClient":
void ourRTSPClient::SetOutputFileName(const std::string &file_name) {
  file_name_ = file_name;
};

const std::string &ourRTSPClient::GetOutputFileName(void) {
  return file_name_;
};

void ourRTSPClient::SetChannel(int channel) { channel_ = channel; };

int ourRTSPClient::GetChannel(void) const { return channel_; };

ourRTSPClient *ourRTSPClient::createNew(UsageEnvironment &env,
                                        char const *rtspURL, int verbosityLevel,
                                        char const *applicationName,
                                        portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName,
                           tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment &env, char const *rtspURL,
                             int verbosityLevel, char const *applicationName,
                             portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName,
                 tunnelOverHTTPPortNum, -1) {
  tcp_flag_ = false;
  has_shut_down = false;
}

ourRTSPClient::~ourRTSPClient() {
  LOGI << "channel " << channel_ << " ~ourRTSPClient()";
  if (!has_shut_down) {
    LOGI << "channel " << channel_ << " ~ourRTSPClient() call shutdownStream";
    shutdownStream(this);
  }

  LOGI << "~ourRTSPClient(), call media pipeline deinit, channel:" << channel_;
  auto media =
      horizon::vision::MediaPipeManager::GetInstance().GetPipeLine()[channel_];
  if (media) {
    media->Stop();
    media->DeInit();
  }
  LOGI << "leave ourRTSPClient::~ourRTSPClient(), channel:" << channel_;
}

void ourRTSPClient::Stop() {
  LOGI << "call ourRTSPClient::Stop(), channel:" << channel_;
  if (!has_shut_down) {
    shutdownStream(this);
    has_shut_down = true;
  }

  LOGI << "ourRTSPClient Stop(), call media pipeline deinit, channel:"
       << channel_;
  auto media =
      horizon::vision::MediaPipeManager::GetInstance().GetPipeLine()[channel_];
  if (media) {
    media->Stop();
    media->DeInit();
  }
  LOGI << "leave call ourRTSPClient::Stop(), channel:" << channel_;
}

#define RTSP_CLIENT_VERBOSITY_LEVEL \
  1  // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount =
    0;  // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

ourRTSPClient *openURL(UsageEnvironment &env, char const *progName,
                       char const *rtspURL, const bool tcp_flag,
                       const std::string &file_name) {
  // Begin by creating a "RTSPClient" object.  Note that there is a separate
  // "RTSPClient" object for each stream that we wish to receive (even if more
  // than stream uses the same "rtsp://" URL).
  ourRTSPClient *rtspClient = ourRTSPClient::createNew(
      env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
  if (rtspClient == NULL) {
    env << "Failed to create a RTSP client for URL \"" << rtspURL
        << "\": " << env.getResultMsg() << "\n";
    return nullptr;
  }
  rtspClient->SetOutputFileName(file_name);
  rtspClient->SetChannel(rtspClientCount);
  rtspClient->SetTCPFlag(tcp_flag);

  env << "Set output file name:" << file_name.c_str() << "\n";

  ++rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the
  // stream. Note that this command - like all RTSP commands - is sent
  // asynchronously; we do not block, waiting for a response. Instead, the
  // following function call returns immediately, and we handle the RTSP
  // response later, from within the event loop:
  rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
  return rtspClient;
}

// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode,
                           char *resultString) {
  do {
    UsageEnvironment &env = rtspClient->envir();  // alias
    StreamClientState &scs =
        (reinterpret_cast<ourRTSPClient *>(rtspClient))->scs;  // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString
          << "\n";
      delete[] resultString;
      break;
    }

    char *const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription;  // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient
          << "Failed to create a MediaSession object from the SDP description: "
          << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient
          << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do
    // this by iterating over the session's 'subsessions', calling
    // "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command,
    // on each one. (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    // ad
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}

static unsigned getBufferSize_(UsageEnvironment &env, int bufOptName,
                               int socket) {
  int curSize = 0;
  int optlen = sizeof(int);
  // SOCKLEN_T sizeSize = sizeof curSize;
  if (getsockopt(socket, SOL_SOCKET, bufOptName,
                 reinterpret_cast<char *>(&curSize),
                 reinterpret_cast<socklen_t *>(&optlen)) < 0) {
    LOGE << "rtsp getBufferSize error!!!";
    return 0;
  }

  return curSize;
}

unsigned getReceiveBufferSize_(UsageEnvironment &env, int socket) {
  return getBufferSize_(env, SO_RCVBUF, socket);
}

static unsigned setBufferTo_(UsageEnvironment &env, int bufOptName, int socket,
                             unsigned requestedSize) {
  SOCKLEN_T sizeSize = sizeof requestedSize;
  setsockopt(socket, SOL_SOCKET, bufOptName,
             reinterpret_cast<char *>(&requestedSize), sizeSize);

  // Get and return the actual, resulting buffer size:
  return getBufferSize_(env, bufOptName, socket);
}

unsigned setReceiveBufferTo_(UsageEnvironment &env, int socket,
                             unsigned requestedSize) {
  return setBufferTo_(env, SO_RCVBUF, socket, requestedSize);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP,
// change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient *rtspClient) {
  UsageEnvironment &env = rtspClient->envir();  // alias
  StreamClientState &scs =
      (reinterpret_cast<ourRTSPClient *>(rtspClient))->scs;  // alias
  bool tcp_flag = (reinterpret_cast<ourRTSPClient *>(rtspClient))->GetTCPFlag();

  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession
          << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(
          rtspClient);  // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession
          << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
        env << "client port " << scs.subsession->clientPortNum();
      } else {
        env << "client ports " << scs.subsession->clientPortNum() << "-"
            << scs.subsession->clientPortNum() + 1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False,
                                   tcp_flag);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY"
  // command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an
    // appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY,
                                scs.session->absStartTime(),
                                scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void continueAfterSETUP(RTSPClient *rtspClient, int resultCode,
                        char *resultString) {
  do {
    UsageEnvironment &env = rtspClient->envir();  // alias
    ourRTSPClient *our_rtsp_client =
        reinterpret_cast<ourRTSPClient *>(rtspClient);
    StreamClientState &scs = our_rtsp_client->scs;  // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession
          << "\" subsession: " << resultString << "\n";
      break;
    }
    std::string media_name(scs.subsession->mediumName());
    if (media_name.compare("video") != 0) {
      env << *rtspClient << "Skip " << scs.subsession->mediumName()
          << " stream";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession
        << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-"
          << scs.subsession->clientPortNum() + 1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and
    // call "startPlaying()" on it. (This will prepare the data sink to receive
    // data; the actual flow of data from the client won't start happening until
    // later, after we've sent a RTSP "PLAY" command.)
    int pay_load = horizon::vision::RTSP_Payload_NONE;
    if (strcmp(scs.subsession->mediumName(), "video") == 0) {
      if (strcmp(scs.subsession->codecName(), "H264") == 0) {
        pay_load = horizon::vision::RTSP_Payload_H264;
      } else if (strcmp(scs.subsession->codecName(), "H265") == 0) {
        pay_load = horizon::vision::RTSP_Payload_H265;
      } else {
        LOGE << "rtsp recv sdp video, unknow codee name:"
             << scs.subsession->codecName();
        shutdownStream(rtspClient);
      }
    } else if (strcmp(scs.subsession->mediumName(), "audio") == 0) {
      LOGE << "channel: " << our_rtsp_client->GetChannel()
           << "recv audio media";
      break;
      if (strcmp(scs.subsession->codecName(), "PCMA") == 0) {
        pay_load = horizon::vision::RTSP_Payload_PCMA;
      } else if (strcmp(scs.subsession->codecName(), "PCMU") == 0) {
        pay_load = horizon::vision::RTSP_Payload_PCMU;
      } else {
        LOGE << "rtsp recv sdp audio, unknow codee name:"
             << scs.subsession->codecName();
        shutdownStream(rtspClient);
      }
    } else {
      LOGE << "rtsp recv sdp info, unknow codee name:"
           << scs.subsession->codecName();
      shutdownStream(rtspClient);
    }

    LOGI << "channel:" << our_rtsp_client->GetChannel()
         << " , rtsp recv decode strem:" << scs.subsession->codecName()
         << ", resolution:" << scs.subsession->videoWidth() << ", "
         << scs.subsession->videoHeight();

    int width = scs.subsession->videoWidth();
    int height = scs.subsession->videoHeight();
    if (width == 0) {
      width = 1920;
      height = 1080;
      LOGW << "channel:" << our_rtsp_client->GetChannel()
           << " recv video width is 0 from sdp, analytics resolution from sps";
#if 0
      if (strcmp(scs.subsession->codecName(), "H264") == 0) {
        unsigned int num = 0;
        SPropRecord *record = parseSPropParameterSets(
            scs.subsession->fmtp_spropparametersets(), num);
        if (num > 0) {
          SPropRecord sps = record[0];
          // SPropRecord pps = record[1];
          struct get_bit_context objBit;
          memset(&objBit, 0, sizeof(objBit));
          objBit.buf = reinterpret_cast<uint8_t *>(sps.sPropBytes) + 1;
          objBit.buf_size = sps.sPropLength;
          struct SPS objSps;
          memset(&objSps, 0, sizeof(objSps));
          if (h264dec_seq_parameter_set(&objBit, &objSps) == 0) {
            width = h264_get_width(&objSps);
            height = h264_get_height(&objSps);
            LOGW << "sps anlytics get width:" << width << " height:" << height;
          }
        }
        delete[] record;
      }
#endif
    }

    int buffer_size = 200 * 1024;
    int buffer_count = 8;
    if (width > 1920 && height > 1080) {
      buffer_size = 1024 * 1024;
      buffer_count = 6;
      LOGW << "channel:" << our_rtsp_client->GetChannel()
           << " relloc buffer size 1024*1024";
    }

    auto media = horizon::vision::MediaPipeManager::GetInstance()
                     .GetPipeLine()[our_rtsp_client->GetChannel()];

    if (pay_load == horizon::vision::RTSP_Payload_H264) {
      H264Sink *dummy_sink_ptr = H264Sink::createNew(
          env, *scs.subsession, rtspClient->url(), buffer_size, buffer_count);
      LOGI << "rtsp client new H264Sink, channel:"
           << our_rtsp_client->GetChannel();
      dummy_sink_ptr->SetFileName(our_rtsp_client->GetOutputFileName());
      dummy_sink_ptr->SetChannel(our_rtsp_client->GetChannel());
      dummy_sink_ptr->AddPipeLine(media);
      scs.subsession->sink = dummy_sink_ptr;
    } else if (pay_load == horizon::vision::RTSP_Payload_H265) {
      H265Sink *dummy_sink_ptr = H265Sink::createNew(
          env, *scs.subsession, rtspClient->url(), buffer_size, buffer_count);
      LOGI << "rtsp client new H265Sink, channel:"
           << our_rtsp_client->GetChannel();
      dummy_sink_ptr->SetFileName(our_rtsp_client->GetOutputFileName());
      dummy_sink_ptr->SetChannel(our_rtsp_client->GetChannel());
      dummy_sink_ptr->AddPipeLine(media);
      scs.subsession->sink = dummy_sink_ptr;
    } else if (pay_load == horizon::vision::RTSP_Payload_PCMU ||
               pay_load == horizon::vision::RTSP_Payload_PCMA) {
      AudioG711Sink *dummy_sink_ptr =
          AudioG711Sink::createNew(env, *scs.subsession, rtspClient->url());
      LOGI << "rtsp client new G711Sink, channel:"
           << our_rtsp_client->GetChannel();
      dummy_sink_ptr->SetFileName(our_rtsp_client->GetOutputFileName());
      dummy_sink_ptr->SetChannel(our_rtsp_client->GetChannel());
      dummy_sink_ptr->AddPipeLine(media);
      scs.subsession->sink = dummy_sink_ptr;
    }

    // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \""
          << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    // our_rtsp_client->SetPayloadType(pay_load);
    media->SetDecodeType(pay_load);
    media->SetDecodeResolution(width, height);

    media->Init();
    media->Start();

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession
        << "\" subsession\n";
    scs.subsession->miscPtr =
        rtspClient;  // a hack to let subsession handler functions get the
                     // "RTSPClient" from the subsession
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this
    // subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler,
                                                    scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient *rtspClient, int resultCode,
                       char *resultString) {
  Boolean success = False;

  do {
    UsageEnvironment &env = rtspClient->envir();  // alias
    StreamClientState &scs =
        (reinterpret_cast<ourRTSPClient *>(rtspClient))->scs;  // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString
          << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration
    // (if the stream does not already signal its end using a RTCP "BYE").  This
    // is optional.  If, instead, you want to keep the stream active - e.g., so
    // you can later 'seek' back within it and do another RTSP "PLAY" - then you
    // can omit this code. (Alternatively, if you don't want to receive the
    // entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop =
          2;  // number of seconds extra to delay, after the stream's expected
              // duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(
          uSecsToDelay, (TaskFunc *)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    //{{
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession *subsession;
    while ((subsession = iter.next()) != NULL) {
      if (subsession->rtpSource() != NULL) {
        unsigned const thresh = 1000000;  // 1 second
        subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);
        int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
        int curBufferSize = getReceiveBufferSize_(env, socketNum);
        int newBufferSize = 200 * 1024;
        if (curBufferSize < newBufferSize) {
          newBufferSize = setReceiveBufferTo_(env, socketNum, newBufferSize);
        }
      }
    }
    //}}

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
  }
}

// Implementation of the other event handlers:

void subsessionAfterPlaying(void *clientData) {
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession &session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return;  // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void *clientData) {
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)subsession->miscPtr;
  UsageEnvironment &env = rtspClient->envir();  // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession
      << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void *clientData) {
  ourRTSPClient *rtspClient = (ourRTSPClient *)clientData;
  StreamClientState &scs = rtspClient->scs;  // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient *rtspClient, int exitCode) {
  UsageEnvironment &env = rtspClient->envir();  // alias
  StreamClientState &scs =
      (reinterpret_cast<ourRTSPClient *>(rtspClient))->scs;  // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) {
    printf("scs.session\n");
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession *subsession;

    while ((subsession = iter.next()) != NULL) {
      printf("subsession name: %s \n", subsession->mediumName());
      if (subsession->sink != NULL) {
        printf("subsession->sink try close\n");
        Medium::close(subsession->sink);
        subsession->sink = NULL;
        printf("subsession->sink\n");
        if (subsession->rtcpInstance() != NULL) {
          subsession->rtcpInstance()->setByeHandler(
              NULL, NULL);  // in case the server sends a RTCP "BYE" while
                            // handling "TEARDOWN"
        }
        someSubsessionsWereActive = True;
      }
    }
    printf("subsession->sink22\n");
    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the
      // stream. Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
  // Note that this will also cause this stream's "StreamClientState" structure
  // to get reclaimed.

  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you
    // might want to comment this out, and replace it with
    // "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop,
    // and continue running "main()".) exit(exitCode);
  }
}
