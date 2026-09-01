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
 * Run the map-matcher and return per-edge attributes (edge ids, road class,
 * names, lengths, `speed_limit` when baked into the tiles), matching the
 * HTTP service's `/trace_attributes` endpoint shape.
 */
- (NSString*)traceAttributes:(NSString*)request;

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

/**
 * Graph-edge correlation (`locate` action), matching the HTTP service's
 * `/locate` endpoint shape: per input location, the correlated edges/nodes
 * with heading, `percent_along`, and (verbose) full `edge_info`. A point far
 * from any edge yields empty `edges`/`nodes` arrays, not an error.
 */
- (NSString*)locate:(NSString*)request;

@end

#endif /* ValhallaWrapperHeader_h */
