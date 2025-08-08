#pragma once


#define XYH_MIN(x, y) (((x) < (y)) ? (x) : (y))	// 取最大值
#define XYH_MAX(x, y) (((x) > (y)) ? (x) : (y))	// 取最小值
#define XYH_PIN(a, min_value, max_value) XYH_MIN(max_value, XYH_MAX(a, min_value))	// 限幅
#define XYH_VALID_INDEX(idx, range) (((idx) >= 0) && ((idx) < (range)))	//
#define XYH_PIN_INDEX(idx, range) XYH_PIN(idx, 0, (range)-1)	// 限幅索引
#define XYH_SIGN(x) ((((x) > 0.0f) ? 1.0f : 0.0f) + (((x) < 0.0f) ? -1.0f : 0.0f))	// 符号函数