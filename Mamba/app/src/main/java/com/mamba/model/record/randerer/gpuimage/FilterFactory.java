package com.mamba.model.record.randerer.gpuimage;

import android.content.Context;
import android.support.v4.app.NavUtils;

import com.mamba.model.record.randerer.gpuimage.filter.GPUImageBeautyFilter;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;
import com.mamba.model.record.randerer.gpuimage.filter.MagicBlackCatFilter;


/**
 * 滤镜工厂
 */
public class FilterFactory {

    /**
     * 根据滤镜定义创建滤镜
     *
     * @param type
     * @param ctx
     * @return
     */
    public static GPUImageFilter getFilter(Context ctx, FilterType type) {
        Context context = null;
        if (ctx != null) {
            context = ctx.getApplicationContext();
        }
        switch (type) {

            case BEAUTY:
                return new GPUImageBeautyFilter(context);
            case BLACK_CAT:
                return new MagicBlackCatFilter(context);
            case NO_FILTER:
                return null;

            default:
                return new GPUImageFilter();
        }
    }

    /**
     * 根据滤镜获取滤镜的类型
     *
     * @param filter
     * @return
     */
    public static FilterType getFilterType(GPUImageFilter filter) {
        FilterType type = FilterType.NO_FILTER;
        if (filter instanceof GPUImageBeautyFilter) {
            type = FilterType.BEAUTY;
        } else if (filter instanceof MagicBlackCatFilter) {
            type = FilterType.BLACK_CAT;

        }
        return type;
    }


}
