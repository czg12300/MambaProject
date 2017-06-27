#ifndef ST_COMMON_H_
#define ST_COMMON_H_

/// @defgroup st_common st common
/// @brief common definitions for st libs
/// @{


#ifdef _MSC_VER
#	ifdef __cplusplus
#		ifdef ST_STATIC_LIB
#			define ST_SDK_API  extern "C"
#		else
#			ifdef SDK_EXPORTS
#				define ST_SDK_API extern "C" __declspec(dllexport)
#			else
#				define ST_SDK_API extern "C" __declspec(dllimport)
#			endif
#		endif
#	else
#		ifdef ST_STATIC_LIB
#			define ST_SDK_API
#		else
#			ifdef SDK_EXPORTS
#				define ST_SDK_API __declspec(dllexport)
#			else
#				define ST_SDK_API __declspec(dllimport)
#			endif
#		endif
#	endif
#else /* _MSC_VER */
#	ifdef __cplusplus
#		ifdef SDK_EXPORTS
#			define ST_SDK_API extern "C" __attribute__((visibility ("default")))
#		else
#			define ST_SDK_API extern "C"
#		endif
#	else
#		ifdef SDK_EXPORTS
#			define ST_SDK_API __attribute__((visibility ("default")))
#		else
#			define ST_SDK_API
#		endif
#	endif
#endif

/// st handle declearation
typedef void *st_handle_t;

/// st result declearation
typedef int   st_result_t;

#define ST_OK (0)					///< 正常运行
#define ST_E_INVALIDARG (-1)		///< 无效参数
#define ST_E_HANDLE (-2)			///< 句柄错误
#define ST_E_OUTOFMEMORY (-3)		///< 内存不足
#define ST_E_FAIL (-4)				///< 内部错误
#define ST_E_DELNOTFOUND (-5)		///< 定义缺失
#define ST_E_INVALID_PIXEL_FORMAT (-6)	///< 不支持的图像格式
#define ST_E_FILE_NOT_FOUND (-10)	///< 模型文件不存在
#define ST_E_INVALID_FILE_FORMAT (-11)	///< 模型格式不正确，导致加载失败
#define ST_E_INVALID_APPID (-12)	///< 包名错误
#define ST_E_INVALID_AUTH (-13)		///< 加密狗功能不支持
#define ST_E_AUTH_EXPIRE (-14)		///< SDK过期
#define ST_E_FILE_EXPIRE (-15)		///< 模型文件过期
#define ST_E_DONGLE_EXPIRE (-16)	///< 加密狗过期
#define ST_E_ONLINE_AUTH_FAIL (-17)	///< 在线验证失败
#define ST_E_ONLINE_AUTH_TIMEOUT (-18)		///< 在线验证超时


/// st rectangle definition
typedef struct st_rect_t {
	int left;	///< 矩形最左边的坐标
	int top;	///< 矩形最上边的坐标
	int right;	///< 矩形最右边的坐标
	int bottom;	///< 矩形最下边的坐标
} st_rect_t;

/// st float type point definition
typedef struct st_pointf_t {
	float x;	///< 点的水平方向坐标，为浮点数
	float y;	///< 点的竖直方向坐标，为浮点数
} st_pointf_t;

/// st integer type point definition
typedef struct st_pointi_t {
	int x;		///< 点的水平方向坐标，为整数
	int y;		///< 点的竖直方向坐标，为整数
} st_pointi_t;

/// st pixel format definition
typedef enum {
	ST_PIX_FMT_GRAY8,	///< Y    1        8bpp ( 单通道8bit灰度像素 )
	ST_PIX_FMT_YUV420P,	///< YUV  4:2:0   12bpp ( 3通道, 一个亮度通道, 另两个为U分量和V分量通道, 所有通道都是连续的 )
	ST_PIX_FMT_NV12,	///< YUV  4:2:0   12bpp ( 2通道, 一个通道是连续的亮度通道, 另一通道为UV分量交错 )
	ST_PIX_FMT_NV21,	///< YUV  4:2:0   12bpp ( 2通道, 一个通道是连续的亮度通道, 另一通道为VU分量交错 )
	ST_PIX_FMT_BGRA8888,///< BGRA 8:8:8:8 32bpp ( 4通道32bit BGRA 像素 )
	ST_PIX_FMT_BGR888,	///< BGR  8:8:8   24bpp ( 3通道24bit BGR 像素 )
	ST_PIX_FMT_RGBA8888	///< BGRA 8:8:8:8 32bpp ( 4通道32bit RGBA 像素 )
} st_pixel_format;

typedef enum {
	ST_CLOCKWISE_ROTATE_0 = 0,	///< 图像不需要转向
	ST_CLOCKWISE_ROTATE_90 = 1,	///< 图像需要顺时针旋转90度
	ST_CLOCKWISE_ROTATE_180 = 2,///< 图像需要顺时针旋转180度
	ST_CLOCKWISE_ROTATE_270 = 3	///< 图像需要顺时针旋转270度
} st_rotate_type;

/// @brief 供106点使用
typedef struct st_mobile_106_t {
    st_rect_t rect;		///< 代表面部的矩形区域
    float score;		///< 置信度
    st_pointf_t points_array[106];	///< 人脸106关键点的数组
    int yaw;			///< 水平转角，真实度量的左负右正
    int pitch;			///< 俯仰角，真实度量的上负下正
    int roll;			///< 旋转角，真实度量的左负右正
    int eye_dist;		///< 两眼间距
    int ID;				///< faceID
} st_mobile_106_t,*p_st_mobile_106_t;

/// @}
#endif  // INCLUDE_ST_COMMON_H_
