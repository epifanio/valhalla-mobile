package com.valhalla.valhalla

internal interface ValhallaActorProviding {
  fun route(request: String): String

  fun traceRoute(request: String): String

  fun sourcesToTargets(request: String): String

  fun optimizedRoute(request: String): String
}

/**
 * Access with raw unchecked strings to the Valhalla routing engine. This class is available, but
 * not recommended for general use.
 *
 * @property configPath
 */
internal class ValhallaActor(private val configPath: String) : ValhallaActorProviding {
  private val valhallaKotlin = ValhallaKotlin()

  /**
   * Run a route request to the Valhalla routing engine. This assumes your config path is valid,
   * tiles exist and your request string is valid.
   *
   * @param request
   * @return
   */
  override fun route(request: String): String {
    return valhallaKotlin.route(request, configPath)
  }

  /**
   * Run a trace_route (Meili map-matcher) request to the Valhalla routing engine. Same
   * config/tiles assumptions as [route]. Available from the adventure-riding fork (0.8.0+).
   *
   * @param request
   * @return
   */
  override fun traceRoute(request: String): String {
    return valhallaKotlin.traceRoute(request, configPath)
  }

  /**
   * Run a sources_to_targets (time/distance matrix) request. Same config/tiles
   * assumptions as [route]. Available from the adventure-riding fork (0.8.3+).
   *
   * @param request
   * @return
   */
  override fun sourcesToTargets(request: String): String {
    return valhallaKotlin.sourcesToTargets(request, configPath)
  }

  /**
   * Run an optimized_route (traveling-salesman stop reordering) request. Same
   * config/tiles assumptions as [route]. Available from the adventure-riding
   * fork (0.8.3+).
   *
   * @param request
   * @return
   */
  override fun optimizedRoute(request: String): String {
    return valhallaKotlin.optimizedRoute(request, configPath)
  }
}
