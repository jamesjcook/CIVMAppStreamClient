/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *      http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */


package com.amazon.appstream.sampleclient;

import java.util.Locale;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.Toast;

import com.amazon.appstream.AppStreamInterface;
import com.amazon.appstream.DesQuery;
import com.amazon.appstream.HardwareDecoder;
import com.amazon.appstream.KeyRemap;

/**
 * The Activity for the AppStream Example Client on Android.
 *
 * After creating the basic OpenGL surface and starting the
 * AppStream server, the main task of SampleClientActivity is to
 * siphon up all relevant events and send them to the native
 * layer.
 */
public class SampleClientActivity
    extends FragmentActivity
    implements ConnectDialogFragment.ConnectDialogListener,
    android.view.GestureDetector.OnGestureListener,
    DesQuery.DesQueryListener,
    AppStreamInterface.AppStreamListener {

    static {
        System.loadLibrary("stlport_shared");
        System.loadLibrary("avutil");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("swresample");
        System.loadLibrary("XStxClientLibraryShared");
        System.loadLibrary("appstreamsample");
    }

    private static final String TAG = "SampleClientActivity";

    private boolean mStopped = false;

    private GL2JNIView mGlView = null;

    private String mServerAddress = null;
    private String mDESServerAddress = null;
    private boolean mUseAppServer = false;
    private static final String USE_APP_SERVER = "use_app_server";
    private static final String DES_SERVER_ADDRESS = "des_server_address";
    private final String SERVER_ADDRESS = "server_address";
    private final String APP_ID = "appid";
    private final String USER_ID = "userid";

    private ImageButton mKeyboardToggle;
    private boolean mKeyboardActive = false;

    private GestureDetector mGestureDetector = null;
    private int mKeyboardOffset = 0;

    private FrameLayout mActivityRootView;

    private ConnectDialogFragment mConnectDialog = null;
    private boolean mTouchscreenAvailable ;

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        Log.i(TAG, "onNewIntent");
    }

    /**
     * Initialization. Sets up the app and spawns the connection
     * dialog.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.v(TAG, "onCreate");
        mGestureDetector = new GestureDetector(this, this);

        mGestureDetector.setIsLongpressEnabled(false);

        mTouchscreenAvailable =  getPackageManager().hasSystemFeature("android.hardware.touchscreen") ;
        Log.v(TAG,"Touch screen available: "+mTouchscreenAvailable);

        SharedPreferences prefs = getSharedPreferences("main", MODE_PRIVATE);
        if (prefs.contains(SERVER_ADDRESS)) {
            mServerAddress = prefs.getString(SERVER_ADDRESS, null);
        }
        if (prefs.contains(DES_SERVER_ADDRESS)) {
            mDESServerAddress = prefs.getString(DES_SERVER_ADDRESS, null);
        }
        if (prefs.contains(USE_APP_SERVER)) {
            mUseAppServer = prefs.getBoolean(USE_APP_SERVER, false);
        }
        if (prefs.contains(APP_ID)) {
            mAppId = prefs.getString(APP_ID, null);
        }
        if (prefs.contains(USER_ID)) {
            mUserId = prefs.getString(USER_ID, null);
        }

        requestWindowFeature(Window.FEATURE_NO_TITLE);
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.v(TAG, "onStart");

        AppStreamInterface.setListener(this);

        setContentView(R.layout.activity_sample_client);

        attemptEnableHardwareDecode();

        openConnectDialog(null);
    }

    HardwareDecoder mHardwareDecoder = null;

    private void attemptEnableHardwareDecode() {
        if (mHardwareDecoder == null) {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN) {
                Log.i(TAG, "JellyBean or higher: Using Hardware Decoder");
                try {
                    mHardwareDecoder = new HardwareDecoder("video/avc", 1280, 720);
                } catch (RuntimeException e) {
                    Log.w(TAG, "Never mind. Can't create hardware decoder: " + e.getMessage());
                }
            }
        }
        AppStreamInterface.setHardwareDecoder(mHardwareDecoder);
    }

    private void disableHardwareDecode() {
        AppStreamInterface.setHardwareDecoder(null);
    }

    boolean mFnVisible = false;
    boolean mArrowBarVisible = false;

    Runnable mClearFullscreen = new Runnable() {
        @Override
        public void run() {
            Log.i(TAG, "Clearing Fullscreen");
            if (getWindow() != null) {
                Log.v(TAG, "Clearing/setting window flags");
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
            }
            mActivityRootView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    };

    /**
     * Switch to the "game" view. Sets the content view to the one
     * for your game; in the example, the content view contains only
     * a keyboard icon.
     */
    public void showGame() {
        setContentView(R.layout.game);

        mActivityRootView = (FrameLayout)findViewById(R.id.outer_frame);

		// If we have a touchscreen, then configure all the buttons.
        if (mTouchscreenAvailable) {
            // This little bit of magic is required to detect when the user hides the keyboard.
            // Unfortunately, the setSystemUiVisibility() has a minimum requirement of Honeycomb (3.0/API11).
            // As such, we have raised the minimum API to 11.
            //
            // A workaround that you could try if you need API10 support is to NOT specify a fullscreen
            // theme for the app. The keyboard detection code below doesn't work when the theme is fullscreen.
            // In fact, you could request a non-fullscreen theme on API10, though you may have to also disable
            // requestWindowFeature(Window.FEATURE_NO_TITLE) in onCreate().
            mActivityRootView.getViewTreeObserver().addOnGlobalLayoutListener(new OnGlobalLayoutListener() {
                @SuppressLint("NewApi")
                @Override
                public void onGlobalLayout() {
                    int heightDiff = mActivityRootView.getRootView().getHeight() - mActivityRootView.getHeight();
                    if (heightDiff > 100) { // if more than 100 pixels, its probably a keyboard...
                        Log.v(TAG, "keyboard found");
                        mKeyboardActive = true;
                        mKeyboardOffset = 0;
                    } else if (mKeyboardActive) {
                        Log.v(TAG, "keyboard not found");
                        mKeyboardActive = false;
                        AppStreamInterface.setKeyboardOffset(0);
                        runOnUiThread(mClearFullscreen);
                    }
                }
            });

            mKeyboardToggle = (ImageButton)findViewById(R.id.keyboard_toggle);
            mKeyboardToggle.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (!mKeyboardActive) {
                        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
                        mActivityRootView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
                    } else {
                        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
                        mActivityRootView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
                    }

                    InputMethodManager imm = (InputMethodManager)SampleClientActivity.this.getSystemService(Context.INPUT_METHOD_SERVICE);
                    imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
                    AppStreamInterface.setKeyboardOffset(0);
                }
            });
        }

        // Keep the screen on when we're visible.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mGlView = new GL2JNIView(getApplication());
        mActivityRootView.addView(mGlView, 0);
    }

    /**
     * Open (or reopen) the connection dialog. This dialog will
     * collect the server information from the user and then call
     * the onDialogConnectClick() function to handle the result.
     *
     * If the result is an error, then this function is called AGAIN
     * to change the state of the dialog from "waiting" to "there
     * was an error."
     *
     * @param error An optional error string to display as part of
     *              the dialog.
     */
    public void openConnectDialog(String error) {

        if (mConnectDialog != null) {
            mConnectDialog.setAddressError(error);
            mConnectDialog.resetProgress();
            return;
        }

        ConnectDialogFragment dialog = new ConnectDialogFragment();
        dialog.setAddress(mServerAddress);
        dialog.setAppID(mAppId);
        dialog.setUserId(mUserId);
        dialog.setAddressError(error);
        dialog.setDESAddress(mDESServerAddress);
        dialog.setUseAppServer(mUseAppServer);
        if (mHardwareDecoder == null) {
            dialog.disableUseHardware();
        }
        dialog.show(getSupportFragmentManager(), "ConnectDialogFragment");

        mConnectDialog = dialog;
    }

    PointerCoords mCoordHolder = new PointerCoords();

    /**
     * A "touch event" includes mouse motion when the mouse button
     * is down.
     */
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {

        if (mKeyboardActive) {
            if (mGestureDetector.onTouchEvent(event)) {
                return true;
            }
        }

        if (super.dispatchTouchEvent(event)) return true;

        int flags = 0;
        if (event.getSource() == InputDevice.SOURCE_TOUCHSCREEN) {
            flags = AppStreamInterface.CET_TOUCH_FLAG;
        }

        event.getPointerCoords(0, mCoordHolder);
        switch (event.getAction()) {
        case MotionEvent.ACTION_MOVE :
            AppStreamInterface.mouseEvent((int)mCoordHolder.x, (int)mCoordHolder.y, flags);
            break;
        case MotionEvent.ACTION_DOWN :
            AppStreamInterface.mouseEvent((int)mCoordHolder.x, (int)mCoordHolder.y, AppStreamInterface.CET_MOUSE_1_DOWN | flags);
            break;
        case MotionEvent.ACTION_UP :
            AppStreamInterface.mouseEvent((int)mCoordHolder.x, (int)mCoordHolder.y, AppStreamInterface.CET_MOUSE_1_UP | flags);
            break;
        }
        return true;
    }

    /**
     * A "generic motion event" includes joystick and mouse motion
     * when the mouse button isn't down. In our simple sample, we're
     * not handling the joystick, but this is where any such code
     * would live.
     *
     * This will only ever be called in HONEYCOMB_MR1 (12) or later, so I'm marking the
     * function using \@TargetApi to allow it to call the super.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event) {
        if (event.getSource() == InputDevice.SOURCE_MOUSE) {
            event.getPointerCoords(0, mCoordHolder);
            switch (event.getAction()) {
            case MotionEvent.ACTION_HOVER_MOVE :
                AppStreamInterface.mouseEvent((int)mCoordHolder.x, (int)mCoordHolder.y, 0); 		break;

            default:
                return super.dispatchGenericMotionEvent(event);
            }
            return true;
        }
        return super.dispatchGenericMotionEvent(event);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mConnectDialog!=null) {
            Log.v(TAG, "onPause: mConnectDialog was non-null");
            mConnectDialog.dismiss();
            mConnectDialog = null;
        }
        
        if (mGlView != null) {
            mGlView.onPause();
        }

        savePrefs();
        AppStreamInterface.pause(true);
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
        stopAppStream();
    }

    public void stopAppStream() {
        if (!mStopped) {
            mStopped = true;
            AppStreamInterface.stop();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mStopped = false;
        if (mGlView != null) {
            mGlView.onResume();
        } else {
            openConnectDialog(null);
        }
        AppStreamInterface.pause(false);
    }

    /**
     * Translate from an onKeyDown() or onKeyUp() message to a
     * Windows virtual key code and send it to the NDK layer.
     *
     * @param[in] msg Message to translate.
     * @param[in] down True if a key down message; false otherwise.
     *
     * @see AppStreamWrapper::keyPress
     */
    public boolean onKey(KeyEvent msg, boolean down) {
        return KeyRemap.handleAndroidKey(msg, down);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent msg) {
        if (onKey(msg, true)) {
            return true; // don't call super; it can translate this key to something else
        }
        // We don't know what this key means, so go ahead and let the OS
        // translate it, in case the translated key *is* something we understand.
        return super.onKeyDown(keyCode, msg);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent msg) {
        if (onKey(msg, false)) {
            return true;
        }
        return super.onKeyUp(keyCode, msg);
    }

    private DesQuery mDesQuery = new DesQuery();

    private String mAppId;

    private String mUserId;

    @Override
    public void onDialogConnectClick(String address, String appid, String userid, boolean hardwareEnabled) {

        Resources r = getResources();

        if (hardwareEnabled) {
            attemptEnableHardwareDecode();
        } else {
            disableHardwareDecode();
        }

        mStopped = false;

        if (address == null || address.isEmpty()) {
            openConnectDialog(r.getString(R.string.no_address));
        } else if (appid == null) {
            if (!address.matches("\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}")) {
                openConnectDialog(r.getString(R.string.invalid_address));
                return;
            }

            mServerAddress = address;
            mUseAppServer = true;

            String url = String.format(Locale.US, "ssm://%s:%d?sessionId=%s", address, 80, "9070-0");
            AppStreamInterface.connect(url);
            AppStreamInterface.newFrame();

            savePrefs();
            return;
        } else if (address != null &&
                   address.matches("^(http[s]?://)?([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6}(:[0-9]+)?(/.*)?$") ||
                   address.matches("^(http[s]?://)?\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}(:[0-9]+)?(/.*)?$")) {
            if ( userid == null || (userid.isEmpty()) ) {
                openConnectDialog(r.getString(R.string.user_id_required));
                return;
            } else if (appid.isEmpty()) {
                openConnectDialog(r.getString(R.string.app_id_required));
                return;
            }
            mUseAppServer = false;
            mDESServerAddress = address;
            mAppId = appid;
            mUserId = userid;
            savePrefs();

            // we've received an entitlement server + path
            mDesQuery.setActivity(this);
            mDesQuery.setListener(this);
            mDesQuery.makeQuery(address, appid, userid);
        } else {
            openConnectDialog(r.getString(R.string.invalid_address));
        }
    }

    private void savePrefs() {
        SharedPreferences prefs = getSharedPreferences("main", MODE_PRIVATE);
        Editor e = prefs.edit();
        if (mServerAddress != null) {
            e.putString(SERVER_ADDRESS, mServerAddress);
        }
        if (mDESServerAddress != null) {
            e.putString(DES_SERVER_ADDRESS, mDESServerAddress);
        }
        if (mUserId != null) {
            e.putString(USER_ID, mUserId);
        }
        if (mAppId != null) {
            e.putString(APP_ID, mAppId);
        }
        e.putBoolean(USE_APP_SERVER, mUseAppServer);
        e.apply();
    }

    @Override
    public void onDesQuerySuccess(String address) {
        AppStreamInterface.connect(address);
        AppStreamInterface.newFrame();
    }

    @Override
    public void onDesQueryFailure(String error) {
        // if we fail, try and try again...
        openConnectDialog(error);
    }

    @Override
    public void onConnectSuccess() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mConnectDialog != null) {
                    mConnectDialog.dismiss();
                    mConnectDialog = null;
                }
                showGame();
            }
        });
    }

    @Override
    public void onErrorMessage(final boolean fatal, final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mStopped) {
                    Log.i(TAG, "Ignoring error during stopped state :" + (fatal ? "fatal" : "non fatal") + ":" + message);
                    return; // ignore errors if we're stopped.
                }
                if (fatal) {
                    // Tell the app it needs to pause.
                    AppStreamInterface.pause(true);
                    ErrorDialogFragment dialog = new ErrorDialogFragment();
                    dialog.setMessage(message);
                    dialog.show(getSupportFragmentManager(), "ErrorDialogFragment");

                    // And finally stop AppStream; kill the interfaces to give us a clean slate.
                    stopAppStream();
                } else {
                    if (mConnectDialog != null) {
                        openConnectDialog(message);
                    } else {
                        Toast toast = Toast.makeText(SampleClientActivity.this, message, Toast.LENGTH_LONG);
                        toast.setGravity(Gravity.RIGHT | Gravity.BOTTOM, 10, 10);
                        toast.show();
                    }
                }
            }
        });
    }


    /**
     * Request an OpenGL frame.
     */
    @Override
    public void newFrame() {
        if (mGlView != null) {
            mGlView.requestRender();
        }
    }

    @Override
    public void onReconnecting(final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (message != null) {
                    openConnectDialog("");
                    mConnectDialog.setReconnectMessage(message);
                } else {
                    if (mConnectDialog!=null) {
                        mConnectDialog.dismiss();
                        mConnectDialog = null;
                    }
                }
            }
        });
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        stopAppStream();
    }
    // GestureDetector interface methods. We only use onScroll, but all
    // must be implemented.
    @Override
    public boolean onDown(MotionEvent e) {
        return false;
    }

    @Override
    public void onShowPress(MotionEvent e) {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
                            float distanceY) {
        if (mKeyboardActive) {
            mKeyboardOffset += (int)distanceY;
            mKeyboardOffset = Math.min(Math.max(0,mKeyboardOffset), mGlView.getHeightDelta());
            AppStreamInterface.setKeyboardOffset(mKeyboardOffset);
            return true;
        }
        return false;
    }

    @Override
    public void onLongPress(MotionEvent e) {
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                           float velocityY) {
        return false;
    }

}
