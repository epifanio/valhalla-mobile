#ifndef ValhallaWrapperHeader_h
#define ValhallaWrapperHeader_h

#import <Foundation/Foundation.h>

@class ValhallaWrapper;

@interface ValhallaWrapper : NSObject {
    @private
    void* _actor;
}

- (instancetype)initWithConfigPath:(NSString*)config_path error:(__autoreleasing NSError **)error;

- (NSString*)route:(NSString*)request;

/**
 * Run Valhalla's Meili map-matcher on the supplied GPS trace and return
 * a routed response (with maneuvers + edge info), matching the HTTP
 * service's `/trace_route` endpoint shape.
 */
- (NSString*)traceRoute:(NSString*)request;

@end

#endif /* ValhallaWrapperHeader_h */
