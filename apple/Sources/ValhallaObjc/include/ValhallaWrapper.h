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

/**
 * Time/distance matrix (`sources_to_targets` action), matching the HTTP
 * service's `/sources_to_targets` endpoint shape.
 *
 * NS_SWIFT_NAME keeps the Valhalla action name intact — Swift's automatic
 * import would otherwise split it into `sources(toTargets:)`.
 */
- (NSString*)sourcesToTargets:(NSString*)request NS_SWIFT_NAME(sourcesToTargets(_:));

/**
 * Traveling-salesman stop reordering (`optimized_route` action), matching
 * the HTTP service's `/optimized_route` endpoint shape. The optimized order
 * is reported via `trip.locations[].original_index`.
 */
- (NSString*)optimizedRoute:(NSString*)request;

@end

#endif /* ValhallaWrapperHeader_h */
