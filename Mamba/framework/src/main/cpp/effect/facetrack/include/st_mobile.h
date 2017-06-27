
#ifndef INCLUDE_STMOBILE_API_ST_MOBILE_H_
#define INCLUDE_STMOBILE_API_ST_MOBILE_H_

#include "st_common.h"
/// @defgroup st_mobile_common st_mobile common
/// @brief Common definitions for st_mobile
/// @{

/// @defgroup st_mobile_track face 106 points track
/// @brief face 106 points tracking interfaces
///
/// This set of interfaces processing face 106 points tracking routines
///
/// @{

/// @brief tracking配置选项，对应st_mobile_tracker_106_create中的config参数，具体配置如下：

// 使用单线程或双线程track.
#define ST_MOBILE_TRACKING_MULTI_THREAD 0x00000000 ///< 多线程, 功耗较多，卡顿较少
#define ST_MOBILE_TRACKING_SINGLE_THREAD 0x00010000 /// < 单线程， 功耗较少，对于性能弱的手机，会偶尔有卡顿现象
// 选择将图像缩小后进行track，最后再将结果处理为源图像对应结果。如果都不选择，直接处理原图。缩小后可提高处理速度。
#define ST_MOBILE_TRACKING_RESIZE_IMG_320W		0x00000001  ///< resize图像为长边320的图像之后再检测，手机用户建议开启该开关，提高处理速度
#define ST_MOBILE_TRACKING_RESIZE_IMG_640W		0x00000002  ///< resize图像为长边640的图像之后再检测
#define ST_MOBILE_TRACKING_RESIZE_IMG_1280W	0x00000004  ///< resize图像为长边1280的图像之后再检测，处理人脸数目较多的图像建议开启该开关，会减慢处理速度，但检测到的人脸数目更多
// 默认tracking配置，使用多线程+320W,可最大限度的提高速度，并减少卡顿
#define ST_MOBILE_TRACKING_DEFAULT_CONFIG ST_MOBILE_TRACKING_MULTI_THREAD|ST_MOBILE_TRACKING_RESIZE_IMG_320W

/// @brief 创建实时人脸106关键点跟踪句柄
/// @param[in] model_path 模型文件的绝对路径或相对路径,例如models/track.tar
/// @param[in] config 配置选项 例如ST_MOBILE_TRACKING_DEFAULT_CONFIG，默认使用双线程跟踪+RESIZE320W，实时视频预览建议使用该配置。
//     使用单线程算法建议选择（ST_MOBILE_TRACKING_SINGLE_THREAD | ST_MOBILE_RESIZE_IMG_320W)
/// @parma[out] handle 人脸跟踪句柄，失败返回NULL
/// @return 成功返回ST_OK, 失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API st_result_t
st_mobile_tracker_106_create(
	const char* model_path,
	unsigned int config,
	st_handle_t* handle
);

/// @brief 设置检测到的最大人脸数目N，持续track已检测到的N个人脸直到人脸数小于N再继续做detect.
/// @param[in] handle 已初始化的关键点跟踪句柄
/// @param[in] max_facecount 设置为1即是单脸跟踪，有效范围为[1, 32]
/// @return 成功返回ST_OK, 失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API
st_result_t st_mobile_tracker_106_set_facelimit(
	st_handle_t handle,
	int max_facecount
);

/// @brief 设置tracker每多少帧进行一次detect.
/// @param[in] handle 已初始化的关键点跟踪句柄
/// @param[in] val  有效范围[1, -)
/// @return 成功返回ST_OK,失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API
st_result_t st_mobile_tracker_106_set_detect_interval(
	st_handle_t handle,
	int val
);

/// @brief 重置实时人脸106关键点跟踪，清空track造成的缓存，重新检测下一帧图像中的人脸并跟踪，建议在两帧图片相差较大时使用
/// @param [in] handle 已初始化的实时目标人脸106关键点跟踪句柄
/// @return 成功返回ST_OK, 失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API st_result_t
st_mobile_tracker_106_reset(
	st_handle_t handle
);

/// @brief 对连续视频帧进行实时快速人脸106关键点跟踪
/// @param handle 已初始化的实时人脸跟踪句柄
/// @param image 用于检测的图像数据
/// @param pixel_format 用于检测的图像数据的像素格式,都支持，不推荐BGRA和BGR，会慢
/// @param image_width 用于检测的图像的宽度(以像素为单位)
/// @param image_height 用于检测的图像的高度(以像素为单位)
/// @param[in] image_stride 用于检测的图像的跨度(以像素为单位)，即每行的字节数；目前仅支持字节对齐的padding，不支持roi
/// @param orientation 视频中人脸的方向
/// @param p_faces_array 检测到的人脸信息数组，api负责分配内存，需要调用st_mobile_release_tracker_result函数释放
/// @param p_faces_count 检测到的人脸数量
/// @return 成功返回ST_OK，失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API st_result_t
st_mobile_tracker_106_track(
	st_handle_t handle,
	const unsigned char *image,
	st_pixel_format pixel_format,
	int image_width,
	int image_height,
	int image_stride,
	st_rotate_type orientation,
	st_mobile_106_t **p_faces_array,
	int *p_faces_count
);

/// @brief 释放实时人脸106关键点跟踪返回结果时分配的空间
/// @param[in] faces_array 跟踪到到的人脸信息数组
/// @param[in] faces_count 跟踪到的人脸数量
ST_SDK_API void
st_mobile_tracker_106_release_result(
	st_mobile_106_t *faces_array,
	int faces_count
);

/// @brief 销毁已初始化的track106句柄
/// @param[in] handle 已初始化的人脸跟踪句柄
ST_SDK_API void
st_mobile_tracker_106_destroy(
	st_handle_t handle
);


/// @brief detect配置开关，对应st_mobile_face_detection_create中的config参数，具体配置如下：

// 选择将图像缩小后进行track，最后再将结果处理为源图像对应结果。如果都不选择，直接处理原图。缩小后可提高处理速度。

#define ST_MOBILE_DETECT_RESIZE_IMG_320W		0x00000001 ///< resize图像为长边320的图像之后再检测，手机用户建议开启该开关，提高处理速度
#define ST_MOBILE_DETECT_RESIZE_IMG_640W		0x00000002  ///< resize图像为长边640的图像之后再检测
#define ST_MOBILE_DETECT_RESIZE_IMG_1280W	0x00000004  ///< resize图像为长边1280的图像之后再检测，处理人脸数目较多的图像建议开启该开关，会减慢处理速度，但检测到的人脸数目更多

// 默认detect配置，直接处理原图,可以最大限度的检测到相应人脸
#define ST_MOBILE_DETECT_DEFAULT_CONFIG		0x00000000


/// @brief 创建人脸检测句柄
/// @param[in] model_path 模型文件的绝对路径或相对路径，例如models/track.tar，可以与track106模型使用相同模型。
/// 模型内文件支持detect; detect+align;detect+align+pose三种模型. detect模型只检测人脸框；detect+align模型检测人脸框和106关键点位置；detect+align+pose检测人脸框、106关键点位置、pose信息
/// @param[in] config 配置选项，例如ST_MOBILE_DETECT_DEFAULT_CONFIG，ST_MOBILE_DETECT_RESIZE_IMG_320W等
/// @parma[out] handle 人脸检测句柄，失败返回NULL
/// @return 成功返回ST_OK, 失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API st_result_t
st_mobile_face_detection_create(
	const char* model_path,
	unsigned int config,
	st_handle_t* handle
);

/// @param[in] handle 已初始化的人脸检测句柄
/// @param[in] image 用于检测的图像数据
/// @param[in] pixel_format 用于检测的图像数据的像素格式,都支持，不推荐BGRA和BGR，会慢
/// @param[in] image_width 用于检测的图像的宽度(以像素为单位)
/// @param[in] image_height 用于检测的图像的高度(以像素为单位)
/// @param[in] image_stride 用于检测的图像的跨度(以像素为单位)，即每行的字节数；目前仅支持字节对齐的padding，不支持roi
/// @param[in] orientation 图像中人脸的方向
/// @param[out] p_faces_array 检测到的人脸信息数组，api负责分配内存，需要调用st_mobile_face_detection_release_result函数释放
/// @param[out] p_faces_count 检测到的人脸数量
/// @return 成功返回ST_OK，失败返回其他错误码,错误码定义在st_common.h 中，如ST_E_FAIL等
ST_SDK_API st_result_t
st_mobile_face_detection_detect(
	st_handle_t handle,
	const unsigned char *image,
	st_pixel_format pixel_format,
	int image_width,
	int image_height,
	int image_stride,
	st_rotate_type orientation,
	st_mobile_106_t **p_faces_array,
	int *p_faces_count
);

/// @brief 释放人脸检测结果
/// @param[in] faces_array 跟踪到到的人脸信息数组
/// @param[in] faces_count 跟踪到的人脸数量
ST_SDK_API void
st_mobile_face_detection_release_result(
	st_mobile_106_t *faces_array,
	int faces_count
);

/// @brief 销毁已初始化的人脸检测句柄
/// @param[in] handle 已初始化的句柄
ST_SDK_API void
st_mobile_face_detection_destroy(
	st_handle_t handle
);

/// 支持的颜色转换格式
typedef enum {
	ST_BGRA_YUV420P = 0,	///< ST_PIX_FMT_BGRA8888到ST_PIX_FMT_YUV420P转换
	ST_BGR_YUV420P = 1,		///< ST_PIX_FMT_BGR888到ST_PIX_FMT_YUV420P转换
	ST_BGRA_NV12 = 2,		///< ST_PIX_FMT_BGRA8888到ST_PIX_FMT_NV12转换
	ST_BGR_NV12 = 3,		///< ST_PIX_FMT_BGR888到ST_PIX_FMT_NV12转换
	ST_BGRA_NV21 = 4,		///< ST_PIX_FMT_BGRA8888到ST_PIX_FMT_NV21转换
	ST_BGR_NV21 = 5,		///< ST_PIX_FMT_BGR888到ST_PIX_FMT_NV21转换
	ST_YUV420P_BGRA = 6,	///< ST_PIX_FMT_YUV420P到ST_PIX_FMT_BGRA8888转换
	ST_YUV420P_BGR = 7,		///< ST_PIX_FMT_YUV420P到ST_PIX_FMT_BGR888转换
	ST_NV12_BGRA = 8,		///< ST_PIX_FMT_NV12到ST_PIX_FMT_BGRA8888转换
	ST_NV12_BGR = 9,		///< ST_PIX_FMT_NV12到ST_PIX_FMT_BGR888转换
	ST_NV21_BGRA = 10,		///< ST_PIX_FMT_NV21到ST_PIX_FMT_BGRA8888转换
	ST_NV21_BGR = 11,		///< ST_PIX_FMT_NV21到ST_PIX_FMT_BGR888转换
	ST_BGRA_GRAY = 12,		///< ST_PIX_FMT_BGRA8888到ST_PIX_FMT_GRAY8转换
	ST_BGR_BGRA = 13,		///< ST_PIX_FMT_BGR888到ST_PIX_FMT_BGRA8888转换
	ST_BGRA_BGR = 14,		///< ST_PIX_FMT_BGRA8888到ST_PIX_FMT_BGR888转换
	ST_YUV420P_GRAY = 15,	///< ST_PIX_FMT_YUV420P到ST_PIX_FMT_GRAY8转换
	ST_NV12_GRAY = 16,		///< ST_PIX_FMT_NV12到ST_PIX_FMT_GRAY8转换
	ST_NV21_GRAY = 17,		///< ST_PIX_FMT_NV21到ST_PIX_FMT_GRAY8转换
	ST_BGR_GRAY = 18,		///< ST_PIX_FMT_BGR888到ST_PIX_FMT_GRAY8转换
	ST_GRAY_YUV420P = 19,	///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_YUV420P转换
	ST_GRAY_NV12 = 20,		///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_NV12转换
	ST_GRAY_NV21 = 21,		///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_NV21转换
	ST_NV12_YUV420P= 22,	///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_NV21转换
	ST_NV21_YUV420P = 23,	///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_NV21转换
	ST_NV21_RGBA = 24,		///< ST_PIX_FMT_NV21到ST_PIX_FMT_RGBA转换
	ST_BGR_RGBA = 25,		///< ST_PIX_FMT_BGR到ST_PIX_FMT_RGBA转换
	ST_BGRA_RGBA = 26,		///< ST_PIX_FMT_BGRA到ST_PIX_FMT_RGBA转换
	ST_RGBA_BGRA = 27,		///< ST_PIX_FMT_RGBA到ST_PIX_FMT_BGRA转换
	ST_GRAY_BGR = 28,		///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_BGR888转换
	ST_GRAY_BGRA = 29,		///< ST_PIX_FMT_GRAY8到ST_PIX_FMT_BGRA8888转换
	ST_NV12_RGBA = 30,		///< ST_PIX_FMT_NV12到ST_PIX_FMT_RGBA8888转换
	ST_NV12_RGB = 31,		///< ST_PIX_FMT_NV12到ST_PIX_FMT_RGB888转换
	ST_RGBA_NV12 = 32,		///< ST_PIX_FMT_RGBA8888到ST_PIX_FMT_NV12转换
	ST_RGB_NV12 = 33,		///< ST_PIX_FMT_RGB888到ST_PIX_FMT_NV12转换
} st_color_convert_type;

/// @brief 进行颜色格式转换, 不建议使用关于YUV420P的转换，速度较慢
/// @param image_src 用于待转换的图像数据
/// @param image_dst 转换后的图像数据
/// @param image_width 用于转换的图像的宽度(以像素为单位)
/// @param image_height 用于转换的图像的高度(以像素为单位)
/// @param type 需要转换的颜色格式
/// @return 正常返回ST_OK，否则返回错误类型
ST_SDK_API st_result_t
st_mobile_color_convert(
	const unsigned char *image_src,
	unsigned char *image_dst,
	int image_width,
	int image_height,
	st_color_convert_type type
);


#endif  // INCLUDE_STMOBILE_API_ST_MOBILE_H_
