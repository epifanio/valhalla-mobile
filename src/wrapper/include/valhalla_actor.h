#ifndef VALHALLAACTOR_H
#define VALHALLAACTOR_H

#include <string>
#include <valhalla/tyr/actor.h>
#include <valhalla/baldr/tilegetter.h>

class ValhallaMobileHttpClient {
public:
    virtual ~ValhallaMobileHttpClient() = default;
    
    /**
     * Makes a synchronous GET request to fetch tile data
     * @param url the URL to fetch
     * @param range_offset optional offset for range requests
     * @param range_size optional size for range requests
     * @return GET_response_t with the response data and status
     */
    virtual valhalla::baldr::tile_getter_t::GET_response_t 
    get(const std::string& url, uint64_t range_offset = 0, uint64_t range_size = 0) = 0;
    
    /**
     * Makes a synchronous HEAD request to fetch response headers
     * @param url the URL to query
     * @param header_mask mask for which headers to retrieve
     * @return HEAD_response_t with the response headers and status
     */
    virtual valhalla::baldr::tile_getter_t::HEAD_response_t 
    head(const std::string& url, valhalla::baldr::tile_getter_t::header_mask_t header_mask) = 0;
};

class ValhallaActor {
private:
    std::unique_ptr<valhalla::tyr::actor_t> actor;
    std::unique_ptr<valhalla::baldr::GraphReader> graph_reader;
public:
    ValhallaActor(const std::string& config_path, ValhallaMobileHttpClient* http_client = nullptr);

    std::string route(const std::string& request);

    /**
     * Run Valhalla's Meili map-matcher on the supplied GPS trace and return
     * a routed result (with maneuvers + edge info), the same shape that
     * `/trace_route` produces on the HTTP service.  Request JSON shape is
     * identical to the HTTP API.
     */
    std::string traceRoute(const std::string& request);

    /**
     * Run the map-matcher and return per-edge attributes (`trace_attributes`
     * action): edge ids, road class, names, lengths and — when present in
     * the tiles — `speed_limit`.  Same request/response JSON shapes as the
     * HTTP `/trace_attributes` endpoint.  This is what powers the offline
     * speed-limit profile: match a computed route's shape, then fold the
     * matched edges into a distance→limit profile client-side.
     */
    std::string traceAttributes(const std::string& request);

    /**
     * Time/distance matrix (`sources_to_targets` action).  Request/response
     * JSON shapes are identical to the HTTP `/sources_to_targets` endpoint.
     */
    std::string sourcesToTargets(const std::string& request);

    /**
     * Traveling-salesman stop reordering (`optimized_route` action): builds a
     * cost matrix across the locations, solves the visiting order, and returns
     * a normal trip whose `trip.locations[].original_index` gives the
     * optimized order.  Same JSON shapes as the HTTP `/optimized_route`
     * endpoint.  First and last locations stay fixed; only the middle is
     * reordered.
     */
    std::string optimizedRoute(const std::string& request);

    /**
     * Graph-edge correlation (`locate` action): answers "which edges/nodes is
     * this point on?" for each input location — correlated lat/lon, heading,
     * `percent_along`, and (with `"verbose":true`) full `edge_info` including
     * names and way id.  A point far from any edge yields empty `edges` /
     * `nodes` arrays, not an error.  Same JSON shapes as the HTTP `/locate`
     * endpoint.  Cheap single-point query — unlike the Meili map-matcher
     * (`traceRoute`), it is suitable for per-fix use during navigation.
     */
    std::string locate(const std::string& request);
};

#endif // VALHALLAACTOR_H
