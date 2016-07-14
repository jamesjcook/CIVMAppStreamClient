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

import android.app.Activity;
import android.app.Dialog;
import android.support.v4.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

/**
 * A manager class for the server_address.xml dialog.
 */
public class ConnectDialogFragment extends DialogFragment {

    /**
     * A listener that receives the results when a user clicks
     * "Connect" in the ConnectDialogFragment. The activity that
     * spawns this dialog is assumed to implement
     * ConnectDialogListener.
     */
    public interface ConnectDialogListener {
        /**
         * Called when the user clicks "Connect."
         *
         * @param address The address the user selected.
         * @param appid If the user selects AppStream Stand-Alone Mode,
         *              then null. Otherwise, the application ID.
         * @param userid If the user selects AppStream Stand-Alone
         *              Mode, then null. Otherwise, the user ID.
         * @param hardwareEnabled True if the dialog is requesting hardware decoding to be enabled.
         */
        public void onDialogConnectClick(String address, String appid, String userid, boolean hardwareEnabled);
    }

    /// The listener that gets notified when the Connect button is clicked.
    private ConnectDialogListener mListener;

    private String mServerAddress = null;
    private String mErrorMessage = null;
    private String mAppId = null;
    private String mUsername = null;
    private String mDESServerAddress = null;
    private boolean mUseAppServer = false;

    /*
     * Set the AppStream Stand-Alone Mode server address.
     *
     * @param address New AppStream Stand-Alone Mode address.
     */
    public void setAddress(String address) {
        mServerAddress = address;
    }
    /*
     * Set the DES server address.
     *
     * @param DESServerAddress
     *               New DES server address.
     */
    public void setDESAddress(String DESServerAddress) {
        mDESServerAddress = DESServerAddress;
    }
    /*
     * Set whether to use the AppStream Stand-Alone Mode connection.
     *
     * @param appStreamStandaloneMode
     *               True to use AppStream Stand-Alone Mode.
     */
    public void setUseAppServer(boolean appStreamStandaloneMode) {
        mUseAppServer  = appStreamStandaloneMode;
    }
    /*
     * Set the app ID.
     *
     * @param appid  New application ID.
     */
    public void setAppID(String appid) {
        mAppId  = appid;
    }
    /*
     * Set the user ID.
     *
     * @param username New user ID.
     */
    public void setUserId(String username) {
        mUsername  = username;
    }

    /*
     * Set the error field.
     *
     * @param error  New error field (default is null -- no error).
     */
    public void setAddressError(String error) {
        mErrorMessage = error;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        mActivity = activity;
        // Verify that the host activity implements the callback interface
        try {
            // Instantiate the NoticeDialogListener so we can send events to the host
            mListener = (ConnectDialogListener)activity;
        } catch (ClassCastException e) {
            // The activity doesn't implement the interface, throw exception
            throw new ClassCastException(activity.toString()
                                         + " must implement NoticeDialogListener");
        }
    }

    public static Drawable mEmpty = new ColorDrawable(0);
    private TextView mAppIdField;
    private TextView mUserIdField;
    private View mSpace1;
    private View mSpace2;
    private View mAppIdTitle;
    private View mUserIdTitle;
    private View mTextEntryFields;
    private TextView mAddressField;
    private TextView mAddressTitle;
    private TextView mErrorMessageField;
    private Activity mActivity;

    private boolean mShowingProgress = false;
    private ProgressBar mProgressBar;
    private View mReconnect;
    private TextView mReconnectMessage;

    private CheckBox mUseHardware;

	private String mReconnectMessageString;

    public void disableUseHardware() {
        if (mUseHardware != null) {
            mUseHardware.setChecked(false);
            mUseHardware.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    Toast.makeText(getActivity(), "Hardware Decode Not Supported", Toast.LENGTH_LONG).show();
                    mUseHardware.setChecked(false);
                }
            });
        }
    }

    /**
     * Stop showing progress indicator.
     */
    public void resetProgress() {
        mShowingProgress = false;
        updateFields();
    }

    /**
     * Update all the fields to be correctly VISIBLE or GONE based
     * on the state flags. Also updates some text that changes on
     * change of state.
     */
    public void updateFields() {
        mReconnect.setVisibility(View.GONE);

        if (mShowingProgress) {
            mProgressBar.setVisibility(View.VISIBLE);
            mTextEntryFields.setVisibility(View.GONE);
        } else {
            int visibility = mUseAppServer ? View.GONE : View.VISIBLE;
            mProgressBar.setVisibility(View.GONE);
            mTextEntryFields.setVisibility(View.VISIBLE);
            mAppIdField.setVisibility(visibility);
            mUserIdField.setVisibility(visibility);
            mSpace1.setVisibility(visibility);
            mSpace2.setVisibility(visibility);
            mAppIdTitle.setVisibility(visibility);
            mUserIdTitle.setVisibility(visibility);
            Resources r = getActivity().getResources();
            if (mUseAppServer) {
                if (mServerAddress != null) {
                    mAddressField.setText(mServerAddress);
                }

                mAddressField.setHint(r.getString(R.string.address_hint_app));
                mAddressTitle.setText(R.string.address_label_app);
            } else {
                if (mDESServerAddress != null) {
                    mAddressField.setText(mDESServerAddress);
                }
                mAddressField.setHint(r.getString(R.string.address_hint));
                mAddressTitle.setText(R.string.address_label);
            }

            if (mErrorMessage == null) {
                mErrorMessageField.setVisibility(View.GONE);
            } else {
                mErrorMessageField.setText(mErrorMessage);
                mErrorMessageField.setVisibility(View.VISIBLE);
            }
        }
    }

    public void reconnecting(String message) {
        mShowingProgress = true;
        updateFields();
        mReconnect.setVisibility(View.VISIBLE);
        mReconnectMessage.setText(message);
    }

    private void onConnect() {
        mShowingProgress = true;
        if (mUseAppServer) {
            mServerAddress = mAddressField.getText().toString();
            mListener.onDialogConnectClick(
                mAddressField.getText().toString(),
                null,
                null, mUseHardware.isChecked());
        } else {
            mDESServerAddress=mAddressField.getText().toString();
            mAppId = mAppIdField.getText().toString();
            mUsername = mUserIdField.getText().toString();

            mListener.onDialogConnectClick(
                mAddressField.getText().toString(),
                mAppIdField.getText().toString(),
                mUserIdField.getText().toString(), mUseHardware.isChecked());
        }

        updateFields();
    }

    /**
     * Create callback; performs initial set-up.
     */
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        mEmpty.setAlpha(0);

        final Dialog connectDialog = new Dialog(getActivity());
        connectDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        connectDialog.setCanceledOnTouchOutside(false);
        connectDialog.setCancelable(false);
        connectDialog.setContentView(R.layout.server_address);
        connectDialog.getWindow().setBackgroundDrawable(mEmpty);

        mAddressTitle = (TextView)connectDialog.findViewById(R.id.address_title);

        mAddressField = (TextView)connectDialog.findViewById(R.id.address);

        mTextEntryFields = connectDialog.findViewById(R.id.text_entry_fields);
        mProgressBar = (ProgressBar)connectDialog.findViewById(R.id.progress_bar);

        mUseHardware = (CheckBox)connectDialog.findViewById(R.id.hardware);
        mUseHardware.setChecked(false);

        final CheckBox useAppServerBox = (CheckBox)connectDialog.findViewById(R.id.appserver);
        useAppServerBox.setChecked(mUseAppServer);
        useAppServerBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

                if (isChecked) {
                    if (!mUseAppServer) {
                        mDESServerAddress = mAddressField.getText().toString();
                    }
                } else {
                    if (mUseAppServer) {
                        mServerAddress = mAddressField.getText().toString();
                    }
                }
                mUseAppServer = isChecked;
                updateFields();
            }
        });

        mAppIdField = (TextView)connectDialog.findViewById(R.id.appid);

        mSpace1 = connectDialog.findViewById(R.id.space1);
        mSpace2 = connectDialog.findViewById(R.id.space2);
        mAppIdTitle = connectDialog.findViewById(R.id.appid_title);
        mUserIdTitle = connectDialog.findViewById(R.id.userid_title);

        if (mAppId != null) {
            mAppIdField.setText(mAppId);
        }

        mUserIdField = (TextView)connectDialog.findViewById(R.id.userid);

        if (mUsername != null) {
            mUserIdField.setText(mUsername);
        }

        mErrorMessageField = (TextView)connectDialog.findViewById(R.id.error_message);

        mReconnect = connectDialog.findViewById(R.id.reconnect_fields);
        mReconnectMessage = (TextView)connectDialog.findViewById(R.id.reconnect_message);

        final Button connectButton = (Button)connectDialog.findViewById(R.id.connect);
        connectButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				onConnect();
			}
		});

		TextView.OnEditorActionListener listener = new TextView.OnEditorActionListener()
	    {
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
				if(actionId == EditorInfo.IME_ACTION_GO) {
					InputMethodManager imm = (InputMethodManager)mActivity.getSystemService(
						      Context.INPUT_METHOD_SERVICE);
					imm.hideSoftInputFromWindow(mUserIdField.getWindowToken(), 0);
					onConnect();
				}
				return true;
			}
		};

		View.OnFocusChangeListener focusListener = new View.OnFocusChangeListener() {

			@Override
			public void onFocusChange(View v, boolean hasFocus) {
				connectButton.setFocusableInTouchMode(false);
			}
		};

		mAppIdField.setOnFocusChangeListener(focusListener);
		mUserIdField.setOnFocusChangeListener(focusListener);
		mUserIdField.setOnFocusChangeListener(focusListener);
        mUserIdField.setOnEditorActionListener(listener);

        updateFields();
        if (mAddressField.getText().length()==0)
        {
        	mAddressField.requestFocus();
        	connectButton.setFocusableInTouchMode(false);
        }
        else
        {
        	connectButton.requestFocus();
        }

        if (mReconnectMessageString!=null) {
        	reconnecting(mReconnectMessageString);
        	mReconnectMessageString = null;
        }

        return connectDialog;
    }
	@Override
	public void onCancel(DialogInterface dialog) {
		super.onCancel(dialog);
		mActivity.finish();
	}
	public void setReconnectMessage(String reconnectMessage) {
		mReconnectMessageString = reconnectMessage;
	}

}
