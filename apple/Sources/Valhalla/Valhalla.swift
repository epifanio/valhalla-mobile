import ValhallaObjc
import ValhallaModels
import ValhallaConfigModels

public protocol ValhallaProviding {
    
    init(_ config: ValhallaConfig) throws
    
    init(configPath: String) throws

    func route(request: RouteRequest) throws -> RouteResponse
}

public final class Valhalla: ValhallaProviding {
    private let actor: ValhallaWrapper?
    private let configPath: String

    public convenience init(_ config: ValhallaConfig) throws {
        let configURL = try ValhallaFileManager.saveConfigTo(config)
        try self.init(configPath: configURL.relativePath)
    }

    public required init(configPath: String) throws {
        do {
            try ValhallaFileManager.injectTzdataIntoLibrary()
        } catch {
            // If you're circumventing this libraries injection, download tzdata.tar and put in your bundle. https://www.iana.org/time-zones
            fatalError("tzdata was not inject into Bundle.main. This can be avoided by including tzdata.tar in your main bundle.")
        }

        self.configPath = configPath
        do {
            self.actor = try ValhallaWrapper(configPath: configPath)
        } catch let error as NSError {
            throw ValhallaError.valhallaError(error.code, error.domain)
        } catch {
            throw ValhallaError.valhallaError(-1, error.localizedDescription)
        }
    }
    
    public func route(request: RouteRequest) throws -> RouteResponse {
        let requestData = try JSONEncoder().encode(request)
        guard let requestStr = String(data: requestData, encoding: .utf8) else {
            throw ValhallaError.encodingNotUtf8("requestStr")
        }
        
        let resultStr = route(rawRequest: requestStr)
        guard let resultData = resultStr.data(using: .utf8) else {
            throw ValhallaError.encodingNotUtf8("resultData")
        }
        
        if let error = try? JSONDecoder().decode(ValhallaErrorModel.self, from: resultData) {
            throw ValhallaError.valhallaError(error.code, error.message)
        }
        
        return try JSONDecoder().decode(RouteResponse.self, from: resultData)
    }

    public func route(rawRequest request: String) -> String {
        actor!.route(request)
    }

    /**
     * Run Valhalla's Meili map-matcher on the supplied GPS trace and
     * return a routed result (maneuvers + edge info), matching the HTTP
     * service's `/trace_route` endpoint.  The request body is the same
     * JSON shape accepted by the HTTP API.
     *
     * Returns the raw JSON response.  Errors are reported in-band as
     * `{"code": <int>, "message": "<text>"}`.
     */
    public func traceRoute(rawRequest request: String) -> String {
        actor!.traceRoute(request)
    }

    /**
     * Run the map-matcher and return per-edge attributes (`trace_attributes`
     * action): edge ids, road class, names, lengths and — when baked into
     * the tiles — `speed_limit`.  Matches the HTTP service's
     * `/trace_attributes` endpoint; the request body is the same JSON shape.
     *
     * Returns the raw JSON response.  Errors are reported in-band as
     * `{"code": <int>, "message": "<text>"}`.
     */
    public func traceAttributes(rawRequest request: String) -> String {
        actor!.traceAttributes(request)
    }

    /**
     * Time/distance matrix (`sources_to_targets` action), matching the HTTP
     * service's `/sources_to_targets` endpoint.  The request body is the
     * same JSON shape accepted by the HTTP API.
     *
     * Returns the raw JSON response.  Errors are reported in-band as
     * `{"code": <int>, "message": "<text>"}`.
     */
    public func sourcesToTargets(rawRequest request: String) -> String {
        actor!.sourcesToTargets(request)
    }

    /**
     * Traveling-salesman stop reordering (`optimized_route` action),
     * matching the HTTP service's `/optimized_route` endpoint.  Valhalla
     * fixes the first and last location and reorders only the middle; the
     * optimized order is reported via `trip.locations[].original_index`.
     *
     * Returns the raw JSON response.  Errors are reported in-band as
     * `{"code": <int>, "message": "<text>"}`.
     */
    public func optimizedRoute(rawRequest request: String) -> String {
        actor!.optimizedRoute(request)
    }

    /**
     * Graph-edge correlation (`locate` action), matching the HTTP service's
     * `/locate` endpoint.  Per input location it returns the correlated
     * edges/nodes with heading, `percent_along`, and — with
     * `"verbose": true` — full `edge_info` (names, way id).  A point far
     * from any edge yields empty `edges`/`nodes` arrays, not an error.
     * Cheap single-point query, suitable for per-fix use during navigation
     * (unlike the Meili map-matcher behind `traceRoute`).
     *
     * Returns the raw JSON response.  Errors are reported in-band as
     * `{"code": <int>, "message": "<text>"}`.
     */
    public func locate(rawRequest request: String) -> String {
        actor!.locate(request)
    }
}
