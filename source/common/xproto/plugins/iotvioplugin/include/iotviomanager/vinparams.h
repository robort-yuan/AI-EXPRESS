/***************************************************************************
 * COPYRIGHT NOTICE
 * Copyright 2020 Horizon Robotics, Inc.
 * All rights reserved.
 ***************************************************************************/
#ifndef INCLUDE_VIN_PARAMS_H_
#define INCLUDE_VIN_PARAMS_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "hb_mipi_api.h"
#include "hb_vin_api.h"

MIPI_SENSOR_INFO_S SENSOR_4LANE_IMX327_30FPS_12BIT_LINEAR_INFO = {
  .deseEnable = 0,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {},
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 0,
    .fps = 30,
    .resolution = 1097,
    .sensor_addr = 0x36,
    .serial_addr = 0,
    .entry_index = 1,
    .sensor_mode = static_cast<MIPI_SENSOR_MODE_E>(1),
    .reg_width = 16,
    .sensor_name = const_cast<char*>("imx327"),
    .extra_mode = 0,
    .deserial_index = -1,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_4LANE_IMX327_30FPS_12BIT_DOL2_INFO = {
  .deseEnable = 0,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {},
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 0,
    .fps = 30,
    .resolution = 2228,
    .sensor_addr = 0x36,
    .serial_addr = 0,
    .entry_index = 0,
    .sensor_mode = static_cast<MIPI_SENSOR_MODE_E>(2),
    .reg_width = 16,
    .sensor_name = const_cast<char*>("imx327"),
    .extra_mode = 0,
    .deserial_index = -1,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_OS8A10_30FPS_10BIT_LINEAR_INFO = {
  .deseEnable = 0,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {},
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 0,
    .fps = 30,
    .resolution = 2160,
    .sensor_addr = 0x36,
    .serial_addr = 0,
    .entry_index = 0,
    .sensor_mode = static_cast<MIPI_SENSOR_MODE_E>(1),
    .reg_width = 16,
    .sensor_name = const_cast<char*>("os8a10"),
    .extra_mode = 0,
    .deserial_index = -1,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_OS8A10_30FPS_10BIT_DOL2_INFO = {
  .deseEnable = 0,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {},
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 0,
    .fps = 30,
    .resolution = 2160,
    .sensor_addr = 0x36,
    .serial_addr = 0,
    .entry_index = 0,
    .sensor_mode = static_cast<MIPI_SENSOR_MODE_E>(2),
    .reg_width = 16,
    .sensor_name = const_cast<char*>("os8a10"),
    .extra_mode = 0,
    .deserial_index = -1,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_954_INFO = {
  .deseEnable = 1,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {
    .bus_type = 0,
    .bus_num = 4,
    .deserial_addr = 0x3d,
    .deserial_name = const_cast<char*>("s954"),
  },
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 4,
    .fps = 30,
    .resolution = 1080,
    .sensor_addr = 0x10,
    .serial_addr = 0x18,
    .entry_index = 1,
    .sensor_mode = PWL_M,
    .reg_width = 16,
    .sensor_name = const_cast<char*>("ar0233"),
    .extra_mode = 0,
    .deserial_index = 0,
    .deserial_port = 0}
};

MIPI_SENSOR_INFO_S SENSOR_4LANE_AR0233_30FPS_12BIT_1080P_960_INFO = {
  .deseEnable = 1,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {
    .bus_type = 0,
    .bus_num = 4,
    .deserial_addr = 0x30,
    .deserial_name = const_cast<char*>("s960")
  },
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 4,
    .fps = 30,
    .resolution = 1080,
    .sensor_addr = 0x10,
    .serial_addr = 0x18,
    .entry_index = 1,
    .sensor_mode = PWL_M,
    .reg_width = 16,
    .sensor_name = const_cast<char*>("ar0233"),
    .extra_mode = 0,
    .deserial_index = 0,
    .deserial_port = 0}
};

MIPI_SENSOR_INFO_S SENSOR_2LANE_OV10635_30FPS_YUV_720P_954_INFO = {
  .deseEnable = 1,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {
    .bus_type = 0,
    .bus_num = 4,
    .deserial_addr = 0x3d,
    .deserial_name = const_cast<char*>("s954")
  },
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 4,
    .fps = 30,
    .resolution = 720,
    .sensor_addr = 0x40,
    .serial_addr = 0x1c,
    .entry_index = 1,
    .sensor_mode = {},
    .reg_width = 16,
    .sensor_name = const_cast<char*>("ov10635"),
    .extra_mode = 0,
    .deserial_index = 0,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_2LANE_OV10635_30FPS_YUV_720P_960_INFO = {
  .deseEnable = 1,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {
    .bus_type = 0,
    .bus_num = 4,
    .deserial_addr = 0x30,
    .deserial_name = const_cast<char*>("s960")
  },
  .sensorInfo = {
    .port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 4,
    .fps = 30,
    .resolution = 720,
    .sensor_addr = 0x40,
    .serial_addr = 0x1c,
    .entry_index = 1,
    .sensor_mode = {},
    .reg_width = 16,
    .sensor_name = const_cast<char*>("ov10635"),
    .extra_mode = 0,
    .deserial_index = 0,
    .deserial_port = 0
  }
};

MIPI_SENSOR_INFO_S SENSOR_TESTPATTERN_INFO = {
  .deseEnable = {},
  .inputMode = {},
  .deserialInfo = {},
  .sensorInfo = {
    .port = {},
    .dev_port = {},
    .bus_type = {},
    .bus_num = {},
    .fps = {},
    .resolution = {},
    .sensor_addr = {},
    .serial_addr = {},
    .entry_index = {},
    .sensor_mode = {},
    .reg_width = {},
    .sensor_name = const_cast<char*>("virtual"),
  }
};

MIPI_SENSOR_INFO_S SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_INFO = {
  .deseEnable = 0,
  .inputMode = INPUT_MODE_MIPI,
  .deserialInfo = {},
  .sensorInfo = {.port = 0,
    .dev_port = 0,
    .bus_type = 0,
    .bus_num = 4,
    .fps = 30,
    .resolution = 3000,
    .sensor_addr = 0x10,
    .serial_addr = 0,
    .entry_index = 1,
    .sensor_mode = NORMAL_M ,
    .reg_width = 16,
    .sensor_name = const_cast<char*>("s5kgm1sp"),
    .extra_mode = 0,
    .deserial_index = -1,
    .deserial_port = 0
  }
};

MIPI_ATTR_S MIPI_2LANE_OV10635_30FPS_YUV_720P_954_ATTR = {
  .mipi_host_cfg = {
    2,    /* lane */
    0x1e, /* datatype */
    24,   /* mclk */
    1600, /* mipiclk */
    30,   /* fps */
    1280, /* width */
    720,  /*height */
    3207, /* linlength */
    748,  /* framelength */
    30,   /* settle */
    4,
    {0, 1, 2, 3}
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_2LANE_OV10635_30FPS_YUV_720P_960_ATTR = {
  .mipi_host_cfg = {
    2,    /* lane */
    0x1e, /* datatype */
    24,   /* mclk */
    3200, /* mipiclk */
    30,   /* fps */
    1280, /* width  */
    720,  /*height */
    3207, /* linlength */
    748,  /* framelength */
    30,   /* settle */
    4,
    {0, 1, 2, 3}
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_954_ATTR = {
  .mipi_host_cfg = {
    4,    /* lane */
    0x2c, /* datatype */
    24,   /* mclk */
    1224, /* mipiclk */
    30,   /* fps */
    1920, /* width */
    1080, /*height */
    2000, /* linlength */
    1700, /* framelength */
    30,   /* settle */
    4,
    {0, 1, 2, 3}
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_AR0233_30FPS_12BIT_1080P_960_ATTR = {
  .mipi_host_cfg = {
    4,    /* lane */
    0x2c, /* datatype */
    24,   /* mclk */
    3200, /* mipiclk */
    30,   /* fps */
    1920, /* width */
    1080, /*height */
    2000, /* linlength */
    1111, /* framelength */
    30,   /* settle */
    4,
    {0, 1, 2, 3}
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_NORMAL_ATTR = {
  .mipi_host_cfg = {
    4,    /* lane */
    0x2c, /* datatype */
    24,   /* mclk */
    891,  /* mipiclk */
    30,   /* fps */
    1952, /* width */
    1097, /*height */
    2152, /* linlength */
    1150, /* framelength */
    20    /* settle */
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_NORMAL_SENSOR_CLK_ATTR = {
  .mipi_host_cfg =
  {
    4,    /* lane */
    0x2c, /* datatype */
    3713, /* mclk */
    891,  /* mipiclk */
    30,   /* fps */
    1952, /* width */
    1097, /*height */
    2152, /* linlength */
    1150, /* framelength */
    20    /* settle */
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_4LANE_SENSOR_IMX327_30FPS_12BIT_DOL2_ATTR = {
  .mipi_host_cfg =
  {
    4,    /* lane */
    0x2c, /* datatype */
    24,   /* mclk    */
    1782, /* mipiclk */
    30,   /* fps */
    1952, /* width  */
    2228, /*height */
    2152, /* linlength */
    2300, /* framelength */
    20    /* settle */
  },
  .dev_enable = 0 /* mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_ATTR = {
  .mipi_host_cfg =
  {
    4,           /* lane */
    0x2b,        /* datatype */
    24,          /* mclk */
    1440,        /* mipiclk */
    30,          /* fps */
    3840,        /* width */
    2160,        /*height */
    6326,        /* linlength */
    4474,        /* framelength */
    50,          /* settle */
    4,           /*chnnal_num*/
    {0, 1, 2, 3} /*vc */
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_LINEAR_SENSOR_CLK_ATTR = {
  .mipi_host_cfg =
  {
    4,           /* lane */
    0x2b,        /* datatype */
    2400,        /* mclk */
    1440,        /* mipiclk */
    30,          /* fps */
    3840,        /* width  */
    2160,        /*height */
    6326,        /* linlength */
    4474,        /* framelength */
    50,          /* settle */
    4,           /*chnnal_num*/
    {0, 1, 2, 3} /*vc */
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_OS8A10_30FPS_10BIT_DOL2_ATTR = {
  .mipi_host_cfg =
  {
    4,           /* lane */
    0x2b,        /* datatype */
    24,          /* mclk */
    2880,        /* mipiclk */
    30,          /* fps */
    3840,        /* width */
    2160,        /*height */
    5084,        /* linlength */
    4474,        /* framelength */
    20,          /* settle */
    4,           /*chnnal_num*/
    {0, 1, 2, 3} /*vc */
  },
  .dev_enable = 0 /*  mipi dev enable */
};

MIPI_ATTR_S MIPI_SENSOR_S5KGM1SP_30FPS_10BIT_LINEAR_ATTR = {
  .mipi_host_cfg = {
    4,            /* lane */
    0x2b,         /* datatype */
    24,           /* mclk */
    4600,         /* mipiclk */
    30,           /* fps */
    4000,         /* width */
    3000,         /* height */
    5024,         /* linlength */
    3194,         /* framelength */
    30,           /* settle */
    2,            /*chnnal_num*/
    {0, 1}        /*vc */
  },
  .dev_enable = 0  /*  mipi dev enable */
};

VIN_DEV_ATTR_S DEV_ATTR_AR0233_1080P_BASE = {
  .stSize = {
    0,    /*format*/
    1920, /*width*/
    1080, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 1920,
      .height = 1080,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 2880,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_IMX327_LINEAR_BASE = {
  .stSize = {
    0,    /*format*/
    1952, /*width*/
    1097, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 1952,
      .height = 1097,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 2928,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
    .short_maxexp_lines = 0,
    .medium_maxexp_lines = 0,
    .vc_short_seq = 0,
    .vc_medium_seq = 0,
    .vc_long_seq = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_IMX327_DOL2_BASE = {
  .stSize = {
    0,    /*format*/
    1948, /*width*/
    1109, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 2,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 1,
      .set_init_frame_id = 0,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 1952,
      .height = 1097,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 2928,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 2,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
    .short_maxexp_lines = 0,
    .medium_maxexp_lines = 0,
    .vc_short_seq = 0,
    .vc_medium_seq = 0,
    .vc_long_seq = 1,
  },
};

VIN_DEV_ATTR_S DEV_ATTR_OS8A10_LINEAR_BASE = {
  .stSize = {
    0,    /*format*/
    3840, /*width*/
    2160, /*height*/
    1     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 3840,
      .height = 2160,
      .pix_length = 1,
    }
  },
  .outDdrAttr = {
    .stride = 4800,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_OS8A10_DOL2_BASE = {
  .stSize = {
    0,    /*format*/
    3840, /*width*/
    2160, /*height*/
    1     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 2,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 3840,
      .height = 2160,
      .pix_length = 1,
    }
  },
  .outDdrAttr = {
    .stride = 4800,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 2,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_OV10635_YUV_BASE = {
  .stSize = {
    8,    /*format*/
    1280, /*width*/
    720,  /*height*/
    0     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 8,
      .width = 1280,
      .height = 720,
      .pix_length = 0,
    }
  },
  .outDdrAttr = {
    .stride = 1280,
    .buffer_num = 8,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_FEED_BACK_1097P_BASE = {
  .stSize = {
    0,    /*format*/
    1952, /*width*/
    1097, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 1,
    .data = {
      .format = 0,
      .width = 1952,
      .height = 1097,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 2928,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_S5KGM1SP_LINEAR_BASE = {
  .stSize = {
    0,       /*format*/
    4000,    /*width*/
    3000,    /*height*/
    1        /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 4,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 4000,
      .height = 3000,
      .pix_length = 1,
    }
  },
  .outDdrAttr = {
    .stride = 5000,
    .buffer_num = 8,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_YUV422_BASE = {
  .stSize = {
    8,    /*format*/
    1280, /*width*/
    720,  /*height*/
    0     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
      .enable_pattern = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 8,
    .raw_feedback_en = 0,
    .data = {
      .format = 8,
      .width = 1280,
      .height = 720,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 1280,
    .buffer_num = 8,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_1080P_BASE = {
  .stSize = {
    0,    /*format*/
    1920, /*width*/
    1080, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
      .enable_pattern = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 6,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 1920,
      .height = 1080,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 2880,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_4K_BASE = {
  .stSize = {
    0,    /*format*/
    3840, /*width*/
    2160, /*height*/
    1     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
      .enable_pattern = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 8,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 3840,
      .height = 2160,
      .pix_length = 1,
    }
  },
  .outDdrAttr = {
    .stride = 4800,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_S DEV_ATTR_TEST_PATTERN_12M_BASE = {
  .stSize = {
    0,    /*format*/
    4000, /*width*/
    3000, /*height*/
    2     /*pix_length*/
  },
  {
    .mipiAttr = {
      .enable = 1,
      .ipi_channels = 1,
      .ipi_mode = 0,
      .enable_mux_out = 1,
      .enable_frame_id = 1,
      .enable_bypass = 0,
      .enable_line_shift = 0,
      .enable_id_decoder = 0,
      .set_init_frame_id = 1,
      .set_line_shift_count = 0,
      .set_bypass_channels = 1,
      .enable_pattern = 1,
    },
  },
  .DdrIspAttr = {
    .stride = 0,
    .buf_num = 8,
    .raw_feedback_en = 0,
    .data = {
      .format = 0,
      .width = 4000,
      .height = 3000,
      .pix_length = 2,
    }
  },
  .outDdrAttr = {
    .stride = 6000,
    .buffer_num = 10,
  },
  .outIspAttr = {
    .dol_exp_num = 1,
    .enable_dgain = 0,
    .set_dgain_short = 0,
    .set_dgain_medium = 0,
    .set_dgain_long = 0,
  }
};

VIN_DEV_ATTR_EX_S DEV_ATTR_IMX327_MD_BASE = {
  .path_sel = 0,
  .roi_top = 0,
  .roi_left = 0,
  .roi_width = 1280,
  .roi_height = 640,
  .grid_step = 128,
  .grid_tolerance = 10,
  .threshold = 10,
  .weight_decay = 128,
  .precision = 0
};

VIN_DEV_ATTR_EX_S DEV_ATTR_OV10635_MD_BASE = {
  .path_sel = 1,
  .roi_top = 0,
  .roi_left = 0,
  .roi_width = 1280,
  .roi_height = 640,
  .grid_step = 128,
  .grid_tolerance = 10,
  .threshold = 10,
  .weight_decay = 128,
  .precision = 0
};

VIN_PIPE_ATTR_S PIPE_ATTR_OV10635_YUV_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 1280,
    .height = 720,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 0,
  .ispBypassEn = 1,
  .ispAlgoState = 0,
  .bitwidth = 12,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 0,
    .lname = NULL,
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_IMX327_DOL2_BASE = {
  .ddrOutBufNum = 5,
  .frameDepth = 0,
  .snsMode = SENSOR_DOL2_MODE,
  .stSize = {
    .format = 0,
    .width = 1920,
    .height = 1080,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 2,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 12,
  .startX = 0,
  .startY = 12,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/libimx327_linear.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_IMX327_LINEAR_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 1920,
    .height = 1080,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 2,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 12,
  .startX = 0,
  .startY = 12,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/libimx327_linear.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_AR0233_1080P_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_PWL_MODE,
  .stSize = {
    .format = 0,
    .width = 1920,
    .height = 1080,
  },
  .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(1),
  .temperMode = 2,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 12,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/lib_ar0233_pwl.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_OS8A10_LINEAR_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 3840,
    .height = 2160,
  },
  .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(3),
  .temperMode = 3,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 10,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/libos8a10_linear.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_OS8A10_DOL2_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_DOL2_MODE,
  .stSize = {
    .format = 0,
    .width = 3840,
    .height = 2160,
  },
  .cfaPattern = static_cast<VIN_PIPE_CFA_PATTERN_E>(3),
  .temperMode = 2,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 10,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/libos8a10_dol2.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_S5KGM1SP_LINEAR_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 4000,
    .height = 3000,
  },
  .cfaPattern = PIPE_BAYER_GRBG,
  .temperMode = 2,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 10,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 1,
    .lname = const_cast<char*>("/etc/cam/s5kgm1sp_linear.so"),
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_1080P_BASE = {
  .ddrOutBufNum = 4,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 1920,
    .height = 1080,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 0,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 12,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 0,
    .lname = NULL,
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_12M_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 4000,
    .height = 3000,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 0,
  .ispBypassEn = 0,
  .ispAlgoState = 1,
  .bitwidth = 12,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 0,
    .lname = NULL,
  }
};

VIN_PIPE_ATTR_S PIPE_ATTR_TEST_PATTERN_4K_BASE = {
  .ddrOutBufNum = 8,
  .frameDepth = 0,
  .snsMode = SENSOR_NORMAL_MODE,
  .stSize = {
    .format = 0,
    .width = 3840,
    .height = 2160,
  },
  .cfaPattern = PIPE_BAYER_RGGB,
  .temperMode = 3,
  .ispBypassEn = 0,
  .ispAlgoState = 0,
  .bitwidth = 10,
  .startX = 0,
  .startY = 0,
  .calib = {
    .mode = 0,
    .lname = NULL,
  }
};

VIN_DIS_ATTR_S DIS_ATTR_BASE = {
  .picSize = {
    .pic_w = 1919,
    .pic_h = 1079,
  },
  .disPath = {
    .rg_dis_enable = 0,
    .rg_dis_path_sel = 1,
  },
  .disHratio = 65536,
  .disVratio = 65536,
  .xCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 1919,
  },
  .yCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 1079,
  },
  .disBufNum = 8,
};

VIN_DIS_ATTR_S DIS_ATTR_OV10635_BASE = {
  .picSize = {
    .pic_w = 1279,
    .pic_h = 719,
  },
  .disPath = {
    .rg_dis_enable = 0,
    .rg_dis_path_sel = 1,
  },
  .disHratio = 65536,
  .disVratio = 65536,
  .xCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 1279,
  },
  .yCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 719,
  }
};

VIN_DIS_ATTR_S DIS_ATTR_OS8A10_BASE = {
  .picSize = {
    .pic_w = 3839,
    .pic_h = 2159,
  },
  .disPath = {
    .rg_dis_enable = 0,
    .rg_dis_path_sel = 1,
  },
  .disHratio = 65536,
  .disVratio = 65536,
  .xCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 3839,
  },
  .yCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 2159,
  }
};

VIN_DIS_ATTR_S DIS_ATTR_12M_BASE = {
  .picSize = {
    .pic_w = 3999,
    .pic_h = 2999,
  },
  .disPath = {
    .rg_dis_enable = 0,
    .rg_dis_path_sel = 1,
  },
  .disHratio = 65536,
  .disVratio = 65536,
  .xCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 3999,
  },
  .yCrop = {
    .rg_dis_start = 0,
    .rg_dis_end = 2999,
  }
};

VIN_LDC_ATTR_S LDC_ATTR_12M_BASE = {
  .ldcEnable = 0,
  .ldcPath = {
    .rg_y_only = 0,
    .rg_uv_mode = 0,
    .rg_uv_interpo = 0,
    .reserved1 = 0,
    .rg_h_blank_cyc = 32,
    .reserved0 = 0
  },
  .yStartAddr = 524288,
  .cStartAddr = 786432,
  .picSize = {
    .pic_w = 3999,
    .pic_h = 2999,
  },
  .lineBuf = 99,
  .xParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .yParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .offShift = {
    .rg_center_xoff = 0,
    .rg_center_yoff = 0,
  },
  .xWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 3999,
    .reserved0 = 0
  },
  .yWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 2999,
    .reserved0 = 0
  }
};

VIN_LDC_ATTR_S LDC_ATTR_BASE = {
  .ldcEnable = 0,
  .ldcPath = {
    .rg_y_only = 0,
    .rg_uv_mode = 0,
    .rg_uv_interpo = 0,
    .reserved1 = 0,
    .rg_h_blank_cyc = 32,
    .reserved0 = 0
  },
  .yStartAddr = 524288,
  .cStartAddr = 786432,
  .picSize = {
    .pic_w = 1919,
    .pic_h = 1079,
  },
  .lineBuf = 99,
  .xParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .yParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .offShift = {
    .rg_center_xoff = 0,
    .rg_center_yoff = 0,
  },
  .xWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 1919,
    .reserved0 = 0
  },
  .yWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 1079,
    .reserved0 = 0
  }
};

VIN_LDC_ATTR_S LDC_ATTR_OV10635_BASE = {
  .ldcEnable = 0,
  .ldcPath = {
    .rg_y_only = 0,
    .rg_uv_mode = 0,
    .rg_uv_interpo = 0,
    .reserved1 = 0,
    .rg_h_blank_cyc = 32,
    .reserved0 = 0
  },
  .yStartAddr = 524288,
  .cStartAddr = 786432,
  .picSize = {
    .pic_w = 1279,
    .pic_h = 719,
  },
  .lineBuf = 99,
  .xParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .yParam = {
    .rg_algo_param_b = 1,
    .rg_algo_param_a = 1,
  },
  .offShift = {
    .rg_center_xoff = 0,
    .rg_center_yoff = 0,
  },
  .xWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 1279,
    .reserved0 = 0
  },
  .yWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 719,
    .reserved0 = 0
  }
};

VIN_LDC_ATTR_S LDC_ATTR_OS8A10_BASE = {
  .ldcEnable = 0,
  .ldcPath = {
    .rg_y_only = 0,
    .rg_uv_mode = 0,
    .rg_uv_interpo = 0,
    .reserved1 = 0,
    .rg_h_blank_cyc = 32,
    .reserved0 = 0,
  },
  .yStartAddr = 524288,
  .cStartAddr = 786432,
  .picSize = {
    .pic_w = 3839,
    .pic_h = 2159,
  },
  .lineBuf = 99,
  .xParam = {
    .rg_algo_param_b = 3,
    .rg_algo_param_a = 2,
  },
  .yParam = {
    .rg_algo_param_b = 5,
    .rg_algo_param_a = 4,
  },
  .offShift = {
    .rg_center_xoff = 0,
    .rg_center_yoff = 0,
  },
  .xWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 3839,
    .reserved0 = 0
  },
  .yWoi = {
    .rg_start = 0,
    .reserved1 = 0,
    .rg_length = 2159,
    .reserved0 = 0
  }
};
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  // INCLUDE_VIN_PARAMS_H_
