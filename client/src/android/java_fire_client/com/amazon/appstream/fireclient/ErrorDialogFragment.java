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


package com.amazon.appstream.fireclient;

import android.app.Dialog;
import android.support.v4.app.DialogFragment;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

/**
 * A manager class for the fatal_error.xml dialog.
 */
public class ErrorDialogFragment extends DialogFragment {

	protected static final String TAG = "ErrorDialogFragment";
	private String mMessage ;

    /**
     * Set the error message.
     *
     * @param message The error message.
     */
	public void setMessage( String message ) {
		mMessage=message;
	}

    /**
     * Standard initialization. Sets up the dialog to quit the
     * activity on clicking its only button.
     */
	@Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        final Dialog dialog= new Dialog(getActivity());

        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setCancelable(false);
        dialog.getWindow().setBackgroundDrawable(ConnectDialogFragment.mEmpty);
        dialog.setContentView(R.layout.fatal_error);

        final Button signin = (Button)dialog.findViewById(R.id.signin);
        final TextView message = (TextView)dialog.findViewById(R.id.message);

        message.setText(mMessage);

        signin.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				dialog.dismiss();

				((FireClientActivity)getActivity()).openConnectDialog(null);
			}
		});

        return dialog;
    }
}
