package com.jake.mamba;

public interface OnRecorderListener {
    int TYPE_VIDEO=0x01;
    int TYPE_AUDIO=0x02;
    void onStart(long id, int type);

    void onFail(long id, int type);

    void onSuccess(long id, int type, String outFile);
}