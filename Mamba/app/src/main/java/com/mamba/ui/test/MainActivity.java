package com.mamba.ui.test;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ToggleButton;

import com.mamba.R;


public class MainActivity extends AppCompatActivity implements IPlayerCallBack {
    private VideoPlayer player;
    private PlayerView playerView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);
        playerView = (PlayerView) findViewById(R.id.surfaceView);
        ToggleButton btnControl = (ToggleButton) findViewById(R.id.btnControl);
        player = new VideoPlayer(playerView.getHolder().getSurface(),  getIntent().getStringExtra("out_file"));
        player.setCallBack(this);
        btnControl.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (player.isPlaying()) {
                    player.stop();
                } else {
                    player.play();
                }
            }
        });
    }


    @Override
    protected void onDestroy() {
        if (player != null) {
            player.destroy();
        }
        super.onDestroy();
    }

    @Override
    public void videoAspect(final int width, final int height, float time) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                playerView.setAspect((float) width / height);
            }
        });
    }
}