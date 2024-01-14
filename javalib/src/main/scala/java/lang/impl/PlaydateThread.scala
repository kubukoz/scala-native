package java.lang.impl

import scala.scalanative.runtime.NativeThread
import scala.scalanative.unsafe.CInt
import scala.scalanative.unsafe.CQuote
import logFunctions.log

object PlaydateThread extends NativeThread.Companion {
  type Impl = PlaydateThread

  override def create(thread: Thread, stackSize: Long): Impl =
    new PlaydateThread(thread, stackSize)

  override def yieldThread(): Unit = System.exit(11)
}

class PlaydateThread(val thread: Thread, stackSize: Long) extends NativeThread {
  override def interrupt(): Unit = System.exit(12)
  def setPriority(priority: CInt): Unit = System.exit(13)
  def sleep(millis: Long): Unit = System.exit(14)
  def sleepNanos(nanos: Int): Unit = System.exit(15)
  def unpark(): Unit = System.exit(16)
  protected def park(time: Long, isAbsolute: Boolean): Unit = System.exit(17)
}
