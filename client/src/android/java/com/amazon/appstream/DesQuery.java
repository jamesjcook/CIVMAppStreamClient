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


package com.amazon.appstream;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import android.app.Activity;
import android.util.Log;

/**
 * An example of how to set up communication with an entitlement
 * server. The code in this class is designed to connect with
 * the demonstration entitlement server that ships as a sample
 * with AppStream.
 *
 * The entitlement server will return a URL of an available
 * AppStream server. That URL will contain all of the
 * necessary log-in and entitlement information to connect to
 * the AppStream server.
 */
public class DesQuery {

    public interface DesQueryListener {
        public void onDesQuerySuccess(String address);
        public void onDesQueryFailure(String error);
    }

	protected static final String TAG = "DesQuery";

    private DesQueryListener mListener;
	private Activity mActivity;

    public void setActivity( Activity a ) {
    	mActivity = a;
    }

	public void setListener(DesQueryListener listener) {
		mListener = listener;
	}
	public void makeQuery(String address, final String appid_in, final String userid_in) {
		// The following code should produce results similar to this curl:
		//curl --header Authorization:Usernametest --data terminatePrevious=true \
		//  http://snyderer.desktop.amazon.com:8080/api/entitlements/7d799fa2-2f65-4581-97c0-06f3cac33e3e

		// Get rid of extra whitespace
		address = address.trim();
		final String appid = appid_in.trim();
		final String userid = userid_in.trim();
		
		if (!address.startsWith("http"))
		{
			address = "http://" + address + ":8080";
		}

		if (!address.endsWith("/"))
		{
			address += "/";
		}

		// if there's no path, add api/entitlements/
		if (!address.matches("http[s]?://.+/.+"))
		{
			address += "api/entitlements/";
		}

		final String cleanAddress = address;

		Thread httpThread = new Thread() {

			@Override
			public void run() {
				HttpParams httpParameters = new BasicHttpParams();

				// timeout for a connection in ms
				HttpConnectionParams.setConnectionTimeout(httpParameters, 3000);

				// timeout when waiting for data
				HttpConnectionParams.setSoTimeout(httpParameters, 3000);

		        HttpClient client = new DefaultHttpClient();
		        HttpPost request = new HttpPost();
		        request.setParams(httpParameters);
		        String queryURL = cleanAddress+appid;
		        try {
					request.setURI(new URI(queryURL));
				} catch (URISyntaxException e) {

					sendError("Either address or appid isn't in the correct format. Generated URL:"+queryURL+" Error:"+e.getLocalizedMessage());
					return;
				}
		        request.setHeader("Authorization", "Username"+userid );
		        request.setHeader("Content-Type", "application/x-www-form-urlencoded" );

		        List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(1);
		        nameValuePairs.add(new BasicNameValuePair("terminatePrevious", "true"));

		        try {
			        request.setEntity(new UrlEncodedFormEntity(nameValuePairs));
					HttpResponse response = client.execute(request);
                    BufferedReader result =
                            new BufferedReader(
                            new InputStreamReader(
                                response.getEntity().getContent()
                                )
                            );

                    String url="";

                    if (result != null) {
                    	String line = result.readLine();
                    	if (line!=null) {
                    		url = line.trim();
                    	}
                        result.close();
                    }

                    if (response.getStatusLine().getStatusCode() < 200 || response.getStatusLine().getStatusCode() > 201) {
						if (response.getStatusLine().getStatusCode()==503) {
							sendError("All of our streaming servers are currently in use. Please try again in a few minutes. [503]");
						} else {
							sendError(url+" ["+Integer.toString(response.getStatusLine().getStatusCode())+"]");
						}
						return;
					}

		            Log.v(TAG,"Resulting URL: "+url);
                    final String url_final = url;
		    		mActivity.runOnUiThread(new Runnable(){
		    			@Override
		    			public void run() {
		    				Log.v(TAG,"Sending host url "+url_final);
		    				mListener.onDesQuerySuccess(url_final);
                        }
                    });

				} catch (ClientProtocolException e) {
					sendError("Protocol Exception: "+e.getLocalizedMessage());
				} catch (IOException e) {
                    if (e.getLocalizedMessage()!=null) {
                        sendError("Problem With Connection: " + e.getLocalizedMessage());
                    } else {
                        sendError("Problem With Connection: IOException");
                        e.printStackTrace();
                    }
				}
			}
		};

		httpThread.start();
	}

    private void sendError(final String message) {
		mActivity.runOnUiThread(new Runnable(){
			@Override
			public void run() {
				mListener.onDesQueryFailure(message);
            }
        });
	}
}
