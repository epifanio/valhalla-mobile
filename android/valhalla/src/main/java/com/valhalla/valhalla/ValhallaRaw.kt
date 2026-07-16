package com.valhalla.valhalla

/**
 * Raw JSON-in / JSON-out access to the Valhalla engine using a pre-written
 * config file on disk.
 *
 * This mirrors the iOS `Valhalla.route(rawRequest:)` / `traceRoute(rawRequest:)`
 * surface that the FastGIS React Native bridge depends on. The typed
 * [Valhalla] entry point parses requests/responses through Moshi models, which
 * is lossy for callers that already speak raw Valhalla JSON (and shuttle it
 * straight to/from the HTTP API contract). [ValhallaRaw] skips that layer.
 *
 * The caller is responsible for writing a functional Valhalla config JSON
 * (with a valid local `mjolnir.tile_dir` / `tile_extract` / `tile_url`) and
 * passing its absolute path here — the same contract as the C++ engine, which
 * reads the config file directly.
 *
 * @property configPath Absolute filesystem path to a valhalla.json config file.
 */
class ValhallaRaw(configPath: String) {
  private val actor: ValhallaActorProviding = ValhallaActor(configPath)

  /** Run a route request. Raw Valhalla JSON in, raw Valhalla JSON out. */
  fun route(rawRequest: String): String = actor.route(rawRequest)

  /**
   * Run a trace_route (Meili map-matcher) request. Raw Valhalla JSON in/out.
   * Available from the adventure-riding fork (0.8.0+).
   */
  fun traceRoute(rawRequest: String): String = actor.traceRoute(rawRequest)

  /**
   * Run a sources_to_targets (time/distance matrix) request. Raw Valhalla
   * JSON in/out. Available from the adventure-riding fork (0.8.3+).
   */
  fun sourcesToTargets(rawRequest: String): String = actor.sourcesToTargets(rawRequest)

  /**
   * Run an optimized_route (traveling-salesman stop reordering) request.
   * Raw Valhalla JSON in/out. Available from the adventure-riding fork (0.8.3+).
   */
  fun optimizedRoute(rawRequest: String): String = actor.optimizedRoute(rawRequest)
}
