package nl.gbraad.regroove;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class FilePicker {
    private static final String TAG = "Regroove-FilePicker";
    public static final int PICK_FILE_REQUEST = 1;

    public interface FileSelectedListener {
        void onFileSelected(String path);
    }

    private Activity activity;
    private FileSelectedListener listener;

    public FilePicker(Activity activity, FileSelectedListener listener) {
        this.activity = activity;
        this.listener = listener;
    }

    public void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");  // Accept all files (module files don't have audio MIME type)
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        activity.startActivityForResult(Intent.createChooser(intent, "Select module file"), PICK_FILE_REQUEST);
    }

    public void handleActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != PICK_FILE_REQUEST || resultCode != Activity.RESULT_OK) {
            return;
        }

        if (data != null) {
            Uri uri = data.getData();
            if (uri != null) {
                try {
                    // Copy file from content URI to cache
                    InputStream inputStream = activity.getContentResolver().openInputStream(uri);
                    if (inputStream != null) {
                        // Get original filename from URI if possible
                        String filename = "selected_module.mod";
                        File cacheFile = new File(activity.getCacheDir(), filename);
                        FileOutputStream outputStream = new FileOutputStream(cacheFile);

                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = inputStream.read(buffer)) != -1) {
                            outputStream.write(buffer, 0, bytesRead);
                        }

                        inputStream.close();
                        outputStream.close();

                        String path = cacheFile.getAbsolutePath();
                        Log.i(TAG, "File copied to: " + path);
                        if (listener != null) {
                            listener.onFileSelected(path);
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Failed to copy file: " + e.getMessage());
                }
            }
        }
    }
}
