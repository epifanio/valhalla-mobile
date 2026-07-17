import XCTest
import ValhallaModels
import ValhallaConfigModels
@testable import Valhalla

final class TestValhallaWithTar: XCTestCase {
    var defaultConfig: ValhallaConfig!
    
    override func setUp() async throws {
        let tilesTarUrl = Bundle.module.url(forResource: "TestData/valhalla_tiles", withExtension: "tar")!
        defaultConfig = try ValhallaConfig(tileExtractTar: tilesTarUrl)
        
        let encoded = try JSONEncoder().encode(defaultConfig)
        print(String(data: encoded, encoding: .utf8)!)
    }
    
    /// Validate an incorrect configuration (config file not found).
    func testNoConfigFile() throws {
        do {
            let valhalla = try Valhalla(configPath: "missing.json")

            let request = RouteRequest(
                locations: [
                    RoutingWaypoint(lat: 42.5063, lon: 1.5218),
                    RoutingWaypoint(lat: 42.5086, lon: 1.5394)
                ],
                costing: .auto,
                directionsOptions: DirectionsOptions(units: .mi)
            )
        
            let _ = try valhalla.route(request: request)
            XCTFail("route should throw cannot open file missing.json")
        } catch let error as ValhallaError {
            XCTAssertEqual(error, .valhallaError(-1, "Cannot open file missing.json"))
        }
    }

    /// Validate a valhalla error that requires all configuration to be set up properly.
    func testNoSuitableEdges() throws {
        let valhalla = try Valhalla(defaultConfig)

        let request = RouteRequest(
            locations: [
                RoutingWaypoint(lat: 45.843812, lon: -123.768205),
                RoutingWaypoint(lat: 45.869701, lon: -123.766121)
            ],
            costing: .auto,
            directionsOptions: DirectionsOptions(units: .mi)
        )
        
        do {
            let _ = try valhalla.route(request: request)
            XCTFail("route should throw no suitable edges")
        } catch let error as ValhallaError {
            XCTAssertEqual(error, .valhallaError(171, "No suitable edges near location"))
        }
    }

    /// Validate a successful route fetch.
    func testSuccessfulRoute() throws {
        let valhalla = try Valhalla(defaultConfig)

        let request = RouteRequest(
            locations: [
                RoutingWaypoint(lat: 42.5063, lon: 1.5218),
                RoutingWaypoint(lat: 42.5086, lon: 1.5394)
            ],
            costing: .auto,
            directionsOptions: DirectionsOptions(units: .mi)
        )
        
        let response = try valhalla.route(request: request)

        XCTAssertEqual(response.trip.statusMessage, "Found route between points")
        XCTAssertEqual(response.trip.legs.first?.shape.count, 656)
    }

    /// Validate the 0.8.3 raw actions: optimized_route (traveling salesman)
    /// and sources_to_targets (time/distance matrix) against the fixture graph.
    func testOptimizedRouteAndMatrixRaw() throws {
        let valhalla = try Valhalla(defaultConfig)

        // Three stops around Andorra la Vella. Valhalla fixes the first and
        // last location and reorders only the middle.
        let request = """
        {"locations":[{"lat":42.5063,"lon":1.5218},{"lat":42.5086,"lon":1.5394},{"lat":42.5069,"lon":1.5301}],"costing":"auto","directions_options":{"units":"kilometers"}}
        """
        let raw = valhalla.optimizedRoute(rawRequest: request)
        let json = try JSONSerialization.jsonObject(with: raw.data(using: .utf8)!) as! [String: Any]
        XCTAssertNil(json["error_code"], "optimized_route errored: \(raw.prefix(300))")
        let trip = json["trip"] as! [String: Any]
        let locations = trip["locations"] as! [[String: Any]]
        XCTAssertEqual(locations.count, 3)
        let order = locations.compactMap { $0["original_index"] as? Int }
        XCTAssertEqual(order.sorted(), [0, 1, 2])   // every stop visited exactly once
        XCTAssertEqual(order.first, 0)              // first location stays fixed
        XCTAssertEqual(order.last, 2)               // last location stays fixed

        let matrixRequest = """
        {"sources":[{"lat":42.5063,"lon":1.5218}],"targets":[{"lat":42.5086,"lon":1.5394},{"lat":42.5069,"lon":1.5301}],"costing":"auto"}
        """
        let rawMatrix = valhalla.sourcesToTargets(rawRequest: matrixRequest)
        let matrixJson = try JSONSerialization.jsonObject(with: rawMatrix.data(using: .utf8)!) as! [String: Any]
        XCTAssertNil(matrixJson["error_code"], "sources_to_targets errored: \(rawMatrix.prefix(300))")
        let table = matrixJson["sources_to_targets"] as! [[[String: Any]]]
        XCTAssertEqual(table.count, 1)
        XCTAssertEqual(table.first?.count, 2)
        XCTAssertNotNil(table.first?.first?["distance"])
    }
}
