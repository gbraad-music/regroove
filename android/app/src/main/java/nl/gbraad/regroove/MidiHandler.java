package nl.gbraad.regroove;

import android.annotation.SuppressLint;
import android.content.Context;
import android.media.midi.MidiDevice;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiInputPort;
import android.media.midi.MidiOutputPort;
import android.media.midi.MidiManager;
import android.media.midi.MidiReceiver;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import java.io.IOException;
import java.lang.reflect.Method;

public class MidiHandler {
    private static final String TAG = "Regroove-Native";
    private static MidiHandler instance;

    static {
        System.loadLibrary("main");
    }

    private Context context;
    private MidiManager midiManager;
    private MidiDevice currentMidiDevice;
    private MidiInputPort currentInputPort;
    private Object midiDeviceServer;  // MidiDeviceServer accessed via reflection
    private MidiDeviceInfo[] cachedDevices;

    public MidiHandler(Context context) {
        this.context = context;
        instance = this;
    }

    public void initialize() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            Log.w(TAG, "MIDI requires Android 6.0+");
            return;
        }

        midiManager = (MidiManager) context.getSystemService(Context.MIDI_SERVICE);
        if (midiManager == null) {
            Log.e(TAG, "Failed to get MIDI manager");
            return;
        }

        // Create virtual MIDI device
        createVirtualMidiDevice();

        // Register callback for device changes
        midiManager.registerDeviceCallback(new MidiManager.DeviceCallback() {
            @Override
            public void onDeviceAdded(MidiDeviceInfo device) {
                Log.i(TAG, "MIDI device added: " + getMidiDeviceName(device));
                enumerateMidiDevices();
            }

            @Override
            public void onDeviceRemoved(MidiDeviceInfo device) {
                Log.i(TAG, "MIDI device removed: " + getMidiDeviceName(device));
                enumerateMidiDevices();
            }
        }, null);

        // Initial enumeration
        enumerateMidiDevices();
    }

    @SuppressLint("PrivateApi")
    private void createVirtualMidiDevice() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            Log.w(TAG, "Virtual MIDI device requires Android 10+");
            return;
        }

        try {
            // Use reflection to create MidiDeviceServer (API 29+)
            MidiReceiver[] inputPortReceivers = { new MidiReceiver() {
                @Override
                public void onSend(byte[] msg, int offset, int count, long timestamp) {
                    if (count >= 1) {
                        int status = msg[offset] & 0xFF;
                        int data1 = (count >= 2) ? (msg[offset + 1] & 0x7F) : 0;
                        int data2 = (count >= 3) ? (msg[offset + 2] & 0x7F) : 0;
                        nativeMidiMessage(status, data1, data2, (int)(timestamp / 1000000));
                    }
                }
            }};

            // Call MidiManager.createDeviceServer() via reflection
            Method createDeviceServer = MidiManager.class.getMethod(
                "createDeviceServer",
                MidiReceiver[].class,
                int.class,
                Bundle.class,
                Class.forName("android.media.midi.MidiDeviceServer$Callback")
            );

            midiDeviceServer = createDeviceServer.invoke(
                midiManager,
                inputPortReceivers,
                0,
                null,
                null  // No callback
            );

            if (midiDeviceServer != null) {
                Log.i(TAG, "Created virtual MIDI device - other apps can send MIDI to Junglizer");
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to create virtual MIDI device via reflection", e);
        }
    }

    private void enumerateMidiDevices() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M || midiManager == null) {
            return;
        }

        cachedDevices = midiManager.getDevices();
        Log.i(TAG, "Found " + cachedDevices.length + " MIDI devices");

        nativeMidiClearDevices();

        for (int i = 0; i < cachedDevices.length; i++) {
            final MidiDeviceInfo device = cachedDevices[i];
            String name = getMidiDeviceName(device);
            int id = device.getId();

            Log.i(TAG, "MIDI Device " + i + ": " + name + " (id=" + id + ")");
            nativeMidiAddDevice(i, id, name);
        }
    }

    // Called from native code to get device name safely (avoids race conditions)
    public static String getDeviceNameByIndex(int deviceIndex) {
        if (instance == null || instance.cachedDevices == null) {
            return "Port " + deviceIndex;
        }
        if (deviceIndex < 0 || deviceIndex >= instance.cachedDevices.length) {
            return "Port " + deviceIndex;
        }
        return instance.getMidiDeviceName(instance.cachedDevices[deviceIndex]);
    }

    // Called from native code to open a specific MIDI device by index
    public static void openDeviceByIndex(int deviceIndex) {
        if (instance != null) {
            instance.openDeviceByIndexInternal(deviceIndex);
        }
    }

    private void openDeviceByIndexInternal(int deviceIndex) {
        if (cachedDevices == null || deviceIndex < 0 || deviceIndex >= cachedDevices.length) {
            Log.e(TAG, "Invalid device index: " + deviceIndex);
            return;
        }
        Log.i(TAG, "Opening MIDI device at index: " + deviceIndex);
        openMidiDevice(cachedDevices[deviceIndex]);
    }

    // Called from native code to close the current MIDI device
    public static void closeCurrentDevice() {
        if (instance != null) {
            instance.closeMidiDevice();
        }
    }

    private void openMidiDevice(final MidiDeviceInfo deviceInfo) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return;
        }

        midiManager.openDevice(deviceInfo, new MidiManager.OnDeviceOpenedListener() {
            @Override
            public void onDeviceOpened(MidiDevice device) {
                if (device == null) {
                    Log.e(TAG, "Failed to open MIDI device");
                    return;
                }

                // Close previous device
                closeMidiDevice();

                currentMidiDevice = device;

                // Connect to device's output port to receive MIDI messages
                if (deviceInfo.getOutputPortCount() > 0) {
                    MidiOutputPort outputPort = device.openOutputPort(0);
                    if (outputPort != null) {
                        outputPort.connect(new MidiReceiver() {
                            @Override
                            public void onSend(byte[] msg, int offset, int count, long timestamp) {
                                if (count >= 1) {
                                    int status = msg[offset] & 0xFF;
                                    int data1 = (count >= 2) ? (msg[offset + 1] & 0x7F) : 0;
                                    int data2 = (count >= 3) ? (msg[offset + 2] & 0x7F) : 0;

                                    // Forward to native code
                                    nativeMidiMessage(status, data1, data2, (int)(timestamp / 1000000));
                                }
                            }
                        });
                        Log.i(TAG, "Opened MIDI device: " + getMidiDeviceName(deviceInfo));
                        nativeMidiDeviceOpened(deviceInfo.getId());
                    }
                }
            }
        }, null);
    }

    private void closeMidiDevice() {
        if (currentInputPort != null) {
            try {
                currentInputPort.close();
            } catch (IOException e) {
                Log.e(TAG, "Error closing MIDI input port", e);
            }
            currentInputPort = null;
        }

        if (currentMidiDevice != null) {
            try {
                currentMidiDevice.close();
            } catch (IOException e) {
                Log.e(TAG, "Error closing MIDI device", e);
            }
            currentMidiDevice = null;
        }
    }

    private String getMidiDeviceName(MidiDeviceInfo device) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return "Unknown";
        }

        Bundle properties = device.getProperties();
        String manufacturer = properties.getString(MidiDeviceInfo.PROPERTY_MANUFACTURER);
        String product = properties.getString(MidiDeviceInfo.PROPERTY_PRODUCT);
        String name = properties.getString(MidiDeviceInfo.PROPERTY_NAME);

        if (product != null && !product.isEmpty()) {
            if (manufacturer != null && !manufacturer.isEmpty()) {
                return manufacturer + " " + product;
            }
            return product;
        }

        if (name != null && !name.isEmpty()) {
            return name;
        }

        return "MIDI Device " + device.getId();
    }

    public void shutdown() {
        closeMidiDevice();

        if (midiDeviceServer != null) {
            try {
                // Close MidiDeviceServer via reflection
                Method close = midiDeviceServer.getClass().getMethod("close");
                close.invoke(midiDeviceServer);
            } catch (Exception e) {
                Log.e(TAG, "Error closing virtual MIDI device", e);
            }
        }
    }

    // Native methods
    private native void nativeMidiClearDevices();
    private native void nativeMidiAddDevice(int index, int id, String name);
    private native void nativeMidiDeviceOpened(int id);
    private native void nativeMidiMessage(int status, int data1, int data2, int timestamp);
}
