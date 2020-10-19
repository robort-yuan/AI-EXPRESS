//
// Copyright 2017~2020 Horizon Robotics, Inc.
//

#ifndef INCLUDE_CLIENT_ADAPTOR_TYPE_DEF_H_
#define INCLUDE_CLIENT_ADAPTOR_TYPE_DEF_H_

#include <memory>
#include <vector>

namespace horizon {
namespace auto_client_adaptor {

// image foramts
enum class ImageFormat {
  GRAY = 0,
  YV12 = 1,
  JPEG = 2,
  PNG = 3,
  CR12 = 4,
  BAD = 5,
  NV12 = 6,
  NV21 = 7,
  UNKNOWN = 8
};

// type of perception result
enum class PerceptionObjectType {
  Cyclist = 0,
  Person = 1,
  Vehicle = 2,
  VehicleRear = 3,
  ParkingLock = 4,
  ParkingCorner = 5,
  ParkingSlot = 6,
  Unknown = 7
};

// point
struct Point {
  float x{0};
  float y{0};
  float z{0};
  Point() = default;
  Point(float x_, float y_, float z_ = 0) : x(x_), y(y_), z(z_) {}
};

// rect
struct Rect {
  float x1{0};
  float y1{0};
  float x2{0};
  float y2{0};
  Rect() = default;
  Rect(float lt_x, float lt_y, float rb_x, float rb_y)
      : x1(lt_x), y1(lt_y), x2(rb_x), y2(rb_y) {}
};

// 3d box
struct Box3D {
  Point ult;  // up; left; top
  Point ulb;  // up; left; bottom
  Point urt;  // up; right; top
  Point urb;  // up; right; bottom
  Point dlt;  // down; left; top
  Point dlb;  // down; left; bottom
  Point drt;  // down; right; top
  Point drb;  // down; right; bottom
};

// polygon
struct Polygon {
  std::vector<Point> vertexs;
};

// Segmentation
struct Segmentation {
  std::shared_ptr<std::vector<uint8_t>> values;
  int width{0};
  int height{0};
  float score{0};
  int semantics{0};
};

using SegmentationPtr = std::shared_ptr<Segmentation>;

// 2d perception result
struct PerceptionObject {
  PerceptionObjectType type;
  Rect rect;
  Box3D box;
  Polygon polygon;
  float score{0};
  int id{0};
};

using PerceptionObjectPtr = std::shared_ptr<PerceptionObject>;

// image parameters
struct ImageParameter {
  int height = 0;
  int width = 0;
  int frame_id = 0;
  int cam_id = 0;
  uint64_t timestamp = 0;
  ImageFormat format = ImageFormat::UNKNOWN;
};

using ImageParameterPtr = std::shared_ptr<ImageParameter>;
using ImageDataPtr = std::shared_ptr<std::vector<uint8_t>>;

// a channel of visual
struct CameraChannel {
  ImageParameterPtr param;
  ImageDataPtr data;
  std::vector<PerceptionObjectPtr> perception_results;
  std::vector<SegmentationPtr> segment;
};

using CameraChannelPtr = std::shared_ptr<CameraChannel>;

// data frame
struct frame_data_t {
  int frame_id = 0;
  std::vector<CameraChannelPtr> channels;
};

using ClientFrame = std::vector<std::shared_ptr<std::vector<uint8_t>>>;

}  // namespace auto_client_adaptor
}  // namespace horizon

#endif  // INCLUDE_CLIENT_ADAPTOR_TYPE_DEF_H_