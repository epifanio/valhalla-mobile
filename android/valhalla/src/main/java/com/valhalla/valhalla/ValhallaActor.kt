package com.valhalla.valhalla

internal interface ValhallaActorProviding {
  fun route(request: String): String

  fun traceRoute(request: String): String
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
}
