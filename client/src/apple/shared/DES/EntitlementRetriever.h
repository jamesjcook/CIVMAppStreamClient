/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights
 * Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may
 * not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 * http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, express or implied. See the License for
 * the specific language governing permissions and limitations under
 * the License.
 *
 */


#import <Foundation/Foundation.h>


/** 
 Entitlement retriever
 
 This class takes input in the form of an NSDictionary with keys for
 application id, user id, and entitlements service url.
 This provides a simple example of how a developer may choose to 
 validate a user's access to a server application.
 The entitlement retreiver makes an http request to a entitlements server, 
 which in turn provides the client application with an ec2 host to connect to
 in order to begin the session.
 
 */


@interface EntitlementRetriever : NSObject<NSURLConnectionDataDelegate>

// given an dictionary of values, makre quest to server
-(void) makeEntitlementsRequest:(NSDictionary *)userInfo;
// cancel NSUrlconnection in progress
-(void) cancelRequest;

@property (nonatomic,weak) id delegate;

@end


// delegate protocol for notifying the application of request results
@protocol EntitlementRetrieverDelegate <NSObject>

@required
-(void) didFailWithErrorMessage:(NSString*) errorMessage;
-(void) didRetreiveEntitlementUrl:(NSString*) entitlementUrl;

@end
