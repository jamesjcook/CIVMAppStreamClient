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


#import "EntitlementRetriever.h"

@interface EntitlementRetriever()
{
    BOOL connectionInProgress;
    NSMutableData *responseData;
    NSInteger statusCode;
}

@property (nonatomic, strong) NSURLConnection *connection;

@end

@implementation EntitlementRetriever


-(id) init
{
    if(self = [super init])
    {
        connectionInProgress = false;
        
    }
    
    return self;
}


// make request to entitlements service

-(void) makeEntitlementsRequest:(NSDictionary *)userInfo
{
    
    // retreive values from userInfo dict (passed in
    NSString *urlString = [userInfo valueForKey:@"url"];
    NSString *appId = [userInfo valueForKey:@"appid"];
    NSString *userId = [userInfo valueForKey:@"userid"];
    
    //if urlString doesn't start with http (or https) add it to the beginning
    // also assume this URL needs the port appended
    if (![urlString hasPrefix:@"http"]) {
        // default server settings are stored in HostInfo.plist
        NSString *filepath = [[NSBundle mainBundle]
                              pathForResource:@"HostInfo" ofType:@"plist"];
        NSDictionary *hostInfo = [NSDictionary
                                  dictionaryWithContentsOfFile:filepath];
        
        uint16_t serverPort = [[hostInfo valueForKey:@"XStxServerPort"]integerValue];
        
        urlString = [NSString stringWithFormat:@"http://%@:%i", urlString, serverPort];
    }
    
    //if urlString doesn't end with / make it so it does
    if (![urlString hasSuffix:@"/"]) {
        urlString = [urlString stringByAppendingString:@"/"];
    }
    
    //if urlString doesn't have anything after the network location append api/entitlements/
    NSRange searchRange = [urlString rangeOfString:@"http[s]?://.+/.+" options:NSRegularExpressionSearch];
    if (searchRange.location==NSNotFound) {
        urlString = [urlString stringByAppendingString:@"api/entitlements/"];
    }
    
    //Add the appId to the URL
    urlString = [urlString stringByAppendingString:appId];
    
    NSURL *theURL = [NSURL URLWithString:urlString];
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:theURL cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:15.0];
    
    [request setHTTPMethod:@"POST"];
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    
    // username is in header field -- no password, just pass username
    [request setValue:[NSString stringWithFormat:@"Username %@",userId]
                              forHTTPHeaderField:@"Authorization"];
    
    NSString *requestBodyString = @"terminatePrevious=true";
    
    NSData *requestBody = [requestBodyString dataUsingEncoding:NSUTF8StringEncoding];
    
    [request setValue:[NSString stringWithFormat:@"%i",(int)requestBody.length]
             forHTTPHeaderField:@"Content-Length"];
    
    [request setHTTPBody:requestBody];
    
    self.connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];

}

// stop a request in progross
-(void) cancelRequest
{
    [self.connection cancel];
}


#pragma mark URL Connection and URL Connection Data delegate methods
// do not cache the response
- (NSCachedURLResponse *)connection:(NSURLConnection *)connection
                  willCacheResponse:(NSCachedURLResponse*)cachedResponse
{
    return nil;
}

// initialize NSData in order to begin capture of response
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    NSHTTPURLResponse *httpURLResponse = (NSHTTPURLResponse *)response;
    responseData = [[NSMutableData alloc] init];
    statusCode = httpURLResponse.statusCode;
}


// upon complete load of data, notify our delegate
- (void)connectionDidFinishLoading:(NSURLConnection *)urlconnection
{
    if(responseData != nil)
    {
        NSString *responseString = [[NSString alloc] initWithData:responseData encoding:NSUTF8StringEncoding];
        NSLog(@"response: %@",responseString);
        if( statusCode >= 200 && statusCode < 400 )
        {
            [self.delegate didRetreiveEntitlementUrl:responseString];
        }
        else
        {
            NSString *errorString = [ NSString stringWithFormat:@"%@ [%i]", responseString, (int)statusCode];
            [self.delegate didFailWithErrorMessage: errorString];
        }
    }
    else
    {
        NSString *errorString = @"Problem With Connection: No Data";
        [self.delegate didFailWithErrorMessage: errorString];
    }
    
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSLog(@"%@",error.localizedDescription);
    NSString *errorString = [NSString stringWithFormat:@"Problem With Connection: %@", error.localizedDescription];
    [self.delegate didFailWithErrorMessage: errorString];
}

-(void)connection:(NSURLConnection*)connection didReceiveData:(NSData*)data
{
    [responseData appendData:data];
}

@end
