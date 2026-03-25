// Minimal libuv stub for NodeCog kernel unit tests.
//
// The unit tests exercise Stage 0–2 bootstrap and the new hgfs/scheduler
// functions.  Stage 3 (event-loop integration) is intentionally not exercised
// in isolation because it requires a running uv_loop; its correctness is
// verified at integration-test time.
//
// This file provides no-op / minimal implementations of the libuv symbols
// referenced by kern_boot.cc so that the test binary links without the full
// libuv shared library.

#include "deps/uv/include/uv.h"
#include <string.h>
#include <stdlib.h>

// ── Loop ─────────────────────────────────────────────────────────────────────

int uv_loop_init(uv_loop_t *loop)   { (void)loop; return 0; }
int uv_loop_close(uv_loop_t *loop)  { (void)loop; return 0; }
int uv_run(uv_loop_t *loop, uv_run_mode mode) { (void)loop; (void)mode; return 0; }

// ── Handles ──────────────────────────────────────────────────────────────────

void uv_close(uv_handle_t *handle, uv_close_cb close_cb) {
    if (close_cb) close_cb(handle);
}

// ── Timer ────────────────────────────────────────────────────────────────────

int uv_timer_init(uv_loop_t *loop, uv_timer_t *handle) {
    (void)loop; memset(handle, 0, sizeof(*handle)); return 0;
}
int uv_timer_start(uv_timer_t *handle, uv_timer_cb cb,
                   uint64_t timeout, uint64_t repeat) {
    (void)handle; (void)cb; (void)timeout; (void)repeat; return 0;
}
int uv_timer_stop(uv_timer_t *handle) { (void)handle; return 0; }

// ── Prepare ──────────────────────────────────────────────────────────────────

int uv_prepare_init(uv_loop_t *loop, uv_prepare_t *handle) {
    (void)loop; memset(handle, 0, sizeof(*handle)); return 0;
}
int uv_prepare_start(uv_prepare_t *handle, uv_prepare_cb cb) {
    (void)handle; (void)cb; return 0;
}
int uv_prepare_stop(uv_prepare_t *handle) { (void)handle; return 0; }

// ── Idle ─────────────────────────────────────────────────────────────────────

int uv_idle_init(uv_loop_t *loop, uv_idle_t *handle) {
    (void)loop; memset(handle, 0, sizeof(*handle)); return 0;
}
int uv_idle_start(uv_idle_t *handle, uv_idle_cb cb) {
    (void)handle; (void)cb; return 0;
}
int uv_idle_stop(uv_idle_t *handle) { (void)handle; return 0; }
