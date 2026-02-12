package nl.gbraad.regroove;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import org.libsdl.app.SDLActivity;

/**
 * Regroove - Module Player with Effects
 */
public class MainActivity extends SDLActivity {
    private static final String TAG = "Regroove-Native";

    private MidiHandler midiHandler;
    private FilePicker filePicker;

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "main"  // Our libmain.so contains SDL3 + our code
        };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Start foreground service to keep audio running in background
        Intent serviceIntent = new Intent(this, AudioService.class);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(serviceIntent);
        } else {
            startService(serviceIntent);
        }

        // Hide status bar and make fullscreen
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // Android 11+ (API 30+)
            getWindow().setDecorFitsSystemWindows(false);
            getWindow().getInsetsController().hide(android.view.WindowInsets.Type.statusBars());
        } else {
            // Older Android versions
            getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN
            );
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );
        }

        // Initialize MIDI
        midiHandler = new MidiHandler(this);
        midiHandler.initialize();

        // Initialize FilePicker
        filePicker = new FilePicker(this, new FilePicker.FileSelectedListener() {
            @Override
            public void onFileSelected(String path) {
                nativeFileSelected(path);
            }
        });
    }

    public static void openFilePicker() {
        ((MainActivity) mSingleton).filePicker.openFilePicker();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        filePicker.handleActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onResume() {
        super.onResume();

        // Restore fullscreen flags
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().getInsetsController().hide(android.view.WindowInsets.Type.statusBars());
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );
        }
    }

    @Override
    protected void onDestroy() {
        if (midiHandler != null) {
            midiHandler.shutdown();
        }
        super.onDestroy();
    }

    // Native methods
    public static native void nativeFileSelected(String path);
}
