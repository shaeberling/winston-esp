package com.s13g.winston.esp

import com.google.common.flogger.FluentLogger
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.InetAddress
import java.net.SocketException
import java.net.SocketTimeoutException
import java.net.URL
import java.net.UnknownHostException
import java.util.concurrent.Callable
import java.util.concurrent.Executors
import java.util.concurrent.Future

private val log = FluentLogger.forEnclosingClass()

/** Runs a stress test to the specified winston-esp instance while monitoring its health. */
fun main(args: Array<String>) {
  if (args.isEmpty() || args[0].isBlank()) {
    log.atSevere().log("Missing argument. Specify address of the ESP")
    return
  }
  val hostname = args[0]
  log.atInfo().log("About to run stress test against $hostname ...")

  if (!checkHostValidAndReachable(hostname)) {
    return
  }

  // Start stress test by requesting multiple URLs from multiple threads.
  log.atInfo().log("Starting stress test...")

  // These are paths that don't control actuators, so it is safe to run on a life system.
  val readOnlyPaths = setOf("/io/reed/0", "/io/reed/1", "/io/system/time", "/io/system/heap", "/io/system/stats")
  val execs = Executors.newFixedThreadPool(6)

  while (true) {
    val futures = mutableListOf<Future<Boolean>>()
    repeat(50) {
      futures += execs.submit(Callable { makeRequest(hostname, readOnlyPaths.shuffled()[0]) })
    }

    log.atInfo().log("Batch of requests fired. Waiting for results ...")
    var numFail = 0
    var numSuccess = 0
    futures.forEach {
      if (it.get()) {
        numSuccess++
      } else {
        numFail++
      }
    }
    log.atInfo().log("Out of ${numFail + numSuccess} requests, $numFail failed.")
    if (numFail > 0) {
      break
    }
  }
  execs.shutdown()
  // TODO: Make requests terminate in the middle, making them bad requests. This is to catch issues with ports
  // not being closed properly
}

private fun makeRequest(hostname: String, path: String): Boolean {
  return try {
    val response = get("http://$hostname$path", 1000)
    if (response.statusCode !in 200..299) {
      log.atSevere().log("Server responded with bad status: ${response.statusCode}")
      return false
    }
    log.atInfo()
      .log("Server online. Received response code: ${response.statusCode}, took ${response.durationMillis}ms.")
    true
  } catch (ex: SocketTimeoutException) {
    log.atSevere().log("Server not responding. Request timed out.")
    false
  } catch (ex: SocketException) {
    log.atSevere().log("Server unhealthy: '${ex.message}'")
    false
  }
}

private fun get(urlStr: String, timeoutMillis: Int): Response {
  val startMillis = System.currentTimeMillis()
  val url = URL(urlStr)
  val connection = url.openConnection() as HttpURLConnection
  connection.requestMethod = "GET"
  connection.connectTimeout = timeoutMillis
  connection.doInput = true
  val reader = InputStreamReader(connection.inputStream);
  reader.use {
    val responseText = reader.readText()
    val durationMillis = System.currentTimeMillis() - startMillis
    return Response(responseText, connection.responseCode, durationMillis)
  }
}

private fun checkHostValidAndReachable(hostname: String): Boolean {
  try {
    val host = InetAddress.getByName(hostname)
    if (!host.isReachable(1000)) {
      log.atSevere().log("Host '$hostname' is not reachable.")
      return false
    }
  } catch (ex: UnknownHostException) {
    log.atSevere().log("Invalid host. '${ex.message}'")
    return false
  }
  log.atInfo().log("Host valid and reachable")
  return true
}

data class Response(val data: String, val statusCode: Int, val durationMillis: Long) {
}
