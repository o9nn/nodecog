// Test kernel boot functions
#include "src/kern_boot.h"
#include "src/kern_hypergraph_fs.h"
#include "src/kern_scheduler.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// ─── Helpers ─────────────────────────────────────────────────────────────────

#define PASS(msg)   printf("  ✓ " msg "\n")
#define FAIL(msg)   do { fprintf(stderr, "  ✗ " msg "\n"); return 1; } while(0)
#define SECTION(s)  printf("\n[%s]\n", s)

/** Tolerance for floating-point comparisons throughout this test file. */
static const double TOLERANCE = 0.01;

// ─── Bootstrap tests ─────────────────────────────────────────────────────────

static int test_bootstrap(struct ggml_context **ctx_out) {
  SECTION("Stage 0: Bootstrap");

  struct ggml_context* ctx = nullptr;
  int result = kern_boot_stage0(&ctx, 128 * 1024 * 1024);  // 128 MB

  if (result != 0) {
    fprintf(stderr, "  Stage 0 failed: %d\n", result);
    return 1;
  }
  if (ctx == nullptr) FAIL("Stage 0 returned null context");
  PASS("Stage 0 completed");

  // ── Stage 1 ──────────────────────────────────────────────────────────────
  SECTION("Stage 1: Hypergraph Filesystem");

  struct hgfs_config hgfs_cfg = {
    .max_depth              = 16,
    .initial_atom_capacity  = 10000,
    .enable_attention_gc    = true
  };
  result = kern_boot_stage1_init_hypergraph_fs(ctx, &hgfs_cfg);
  if (result != 0) {
    fprintf(stderr, "  Stage 1 failed: %d\n", result);
    return 1;
  }
  PASS("Stage 1 completed");

  // ── Stage 2 ──────────────────────────────────────────────────────────────
  SECTION("Stage 2: Scheduler");

  struct dtesn_config sched_cfg = {
    .reservoir_size            = 1000,
    .tick_rate_us              = 5,
    .sti_threshold             = 0.5,
    .lti_threshold             = 0.3,
    .enable_adaptive_scheduling = true
  };
  struct dtesn_sched_context* sched = nullptr;
  result = kern_boot_stage2_init_scheduler(&sched, &sched_cfg);
  if (result != 0) {
    fprintf(stderr, "  Stage 2 failed: %d\n", result);
    return 1;
  }
  if (sched == nullptr) FAIL("Stage 2 returned null scheduler");
  PASS("Stage 2 completed");

  // ── Version ──────────────────────────────────────────────────────────────
  uint16_t major, minor, patch;
  kern_get_version(&major, &minor, &patch);
  printf("  ✓ Kernel version: %d.%d.%d\n", major, minor, patch);

  // Cleanup (no event loop in this test, so skip Stage 3)
  dtesn_sched_destroy(sched);

  *ctx_out = ctx;
  return 0;
}

// ─── Hypergraph filesystem tests ─────────────────────────────────────────────

static int test_hypergraph_fs(struct ggml_context *ctx) {
  SECTION("hgfs: hypergraph filesystem");

  // Reset global state for a clean run.
  hgfs_reset();

  // ── hgfs_alloc ───────────────────────────────────────────────────────────
  void *nodeA = hgfs_alloc(ctx, 32, 0);
  if (nodeA == nullptr) FAIL("hgfs_alloc returned NULL for valid parameters");
  PASS("hgfs_alloc: returned non-null node A");

  void *nodeB = hgfs_alloc(ctx, 16, 1);
  if (nodeB == nullptr) FAIL("hgfs_alloc returned NULL for node B");
  PASS("hgfs_alloc: returned non-null node B (depth 1)");

  void *nodeC = hgfs_alloc(ctx, 8, 0);
  if (nodeC == nullptr) FAIL("hgfs_alloc returned NULL for node C");
  PASS("hgfs_alloc: returned non-null node C");

  // NULL ctx / zero size should fail.
  if (hgfs_alloc(nullptr, 32, 0) != nullptr) FAIL("hgfs_alloc should fail with null ctx");
  if (hgfs_alloc(ctx, 0, 0)     != nullptr) FAIL("hgfs_alloc should fail with size=0");
  PASS("hgfs_alloc: correctly rejects invalid inputs");

  // ── hgfs_edge ────────────────────────────────────────────────────────────
  int rc = hgfs_edge(ctx, nodeA, nodeB, 1 /*DEPENDS_ON*/, 0.9f);
  if (rc != 0) FAIL("hgfs_edge A→B failed");
  PASS("hgfs_edge: created edge A→B (type=1, w=0.9)");

  rc = hgfs_edge(ctx, nodeA, nodeC, 2 /*EXPORTS*/, 0.5f);
  if (rc != 0) FAIL("hgfs_edge A→C failed");
  PASS("hgfs_edge: created edge A→C (type=2, w=0.5)");

  // Edge with null endpoints should fail.
  if (hgfs_edge(ctx, nullptr, nodeB, 1, 1.0f) != -1)
    FAIL("hgfs_edge should fail with null src");
  if (hgfs_edge(ctx, nodeA, nullptr, 1, 1.0f) != -1)
    FAIL("hgfs_edge should fail with null dst");
  PASS("hgfs_edge: correctly rejects invalid inputs");

  // ── hgfs_query_neighbors ─────────────────────────────────────────────────
  void *results[8] = {};

  // All outgoing neighbours from A (type_filter = 0 → all)
  int n = hgfs_query_neighbors(ctx, nodeA, 0, results, 8);
  if (n != 2) {
    fprintf(stderr, "  Expected 2 neighbours, got %d\n", n);
    FAIL("hgfs_query_neighbors: wrong neighbour count (all types)");
  }
  PASS("hgfs_query_neighbors: found 2 neighbours with type_filter=0");

  // Filtered by type 1 only
  n = hgfs_query_neighbors(ctx, nodeA, 1, results, 8);
  if (n != 1) {
    fprintf(stderr, "  Expected 1 neighbour of type 1, got %d\n", n);
    FAIL("hgfs_query_neighbors: wrong count for type_filter=1");
  }
  if (results[0] != nodeB) FAIL("hgfs_query_neighbors: wrong neighbour for type 1");
  PASS("hgfs_query_neighbors: filtered by type=1 returns nodeB");

  // nodeB has no outgoing edges.
  n = hgfs_query_neighbors(ctx, nodeB, 0, results, 8);
  if (n != 0) FAIL("hgfs_query_neighbors: nodeB should have no outgoing edges");
  PASS("hgfs_query_neighbors: nodeB has 0 outgoing edges");

  // ── hgfs_free + invalidation ──────────────────────────────────────────────
  hgfs_free(ctx, nodeC);
  if (!hgfs_is_freed(nodeC)) FAIL("hgfs_is_freed: nodeC should be marked freed");
  PASS("hgfs_free: nodeC is in free-list");

  // After freeing nodeC, the A→C edge should be gone.
  n = hgfs_query_neighbors(ctx, nodeA, 0, results, 8);
  if (n != 1) {
    fprintf(stderr, "  Expected 1 live neighbour after freeing C, got %d\n", n);
    FAIL("hgfs_query_neighbors: should skip dead edge after hgfs_free");
  }
  PASS("hgfs_query_neighbors: dead edge skipped after hgfs_free(nodeC)");

  // Creating an edge to a freed node should fail.
  if (hgfs_edge(ctx, nodeA, nodeC, 1, 1.0f) != -1)
    FAIL("hgfs_edge should reject freed destination");
  PASS("hgfs_edge: correctly rejects freed node as destination");

  return 0;
}

// ─── Scheduler tests ─────────────────────────────────────────────────────────

static int test_scheduler(void) {
  SECTION("dtesn_sched: task scheduler");

  dtesn_sched_reset();  // clean slate

  // ── Tick count starts at 0 ────────────────────────────────────────────────
  if (dtesn_sched_get_tick_count() != 0) FAIL("tick count should be 0 initially");
  PASS("dtesn_sched_get_tick_count: starts at 0");

  // ── Tick on empty queue is a no-op ────────────────────────────────────────
  dtesn_sched_tick();
  if (dtesn_sched_get_tick_count() != 0) FAIL("tick on empty queue should not increment counter");
  PASS("dtesn_sched_tick: no-op on empty queue");

  // ── Enqueue tasks ─────────────────────────────────────────────────────────
  int dummy_a = 1, dummy_b = 2, dummy_c = 3;

  if (dtesn_sched_enqueue_task(&dummy_a, 1.0) != 0) FAIL("enqueue task A failed");
  if (dtesn_sched_enqueue_task(&dummy_b, 5.0) != 0) FAIL("enqueue task B failed");
  if (dtesn_sched_enqueue_task(&dummy_c, 3.0) != 0) FAIL("enqueue task C failed");
  PASS("dtesn_sched_enqueue_task: enqueued 3 tasks");

  // ── Tick processes highest-priority task first ────────────────────────────
  dtesn_sched_tick();  // should process B (priority 5.0)
  if (dtesn_sched_get_tick_count() != 1)
    FAIL("tick count should be 1 after first tick");
  PASS("dtesn_sched_tick: tick count incremented to 1");

  dtesn_sched_tick();  // should process C (priority 3.0)
  dtesn_sched_tick();  // should process A (priority 1.0)
  if (dtesn_sched_get_tick_count() != 3)
    FAIL("tick count should be 3 after processing all tasks");
  PASS("dtesn_sched_tick: tick count incremented to 3 after 3 ticks");

  // ── Queue exhausted ───────────────────────────────────────────────────────
  dtesn_sched_tick();
  if (dtesn_sched_get_tick_count() != 3)
    FAIL("tick on empty queue should not change tick count");
  PASS("dtesn_sched_tick: tick count stable when queue empty");

  // ── Enqueue full capacity ─────────────────────────────────────────────────
  dtesn_sched_reset();
  for (uint32_t i = 0; i < DTESN_MAX_TASKS; ++i) {
    if (dtesn_sched_enqueue_task(nullptr, (double)i) != 0) {
      fprintf(stderr, "  Enqueue failed at i=%u\n", i);
      FAIL("enqueue failed before queue was full");
    }
  }
  if (dtesn_sched_enqueue_task(nullptr, 99.0) != -1)
    FAIL("enqueue should return -1 when queue is full");
  PASS("dtesn_sched_enqueue_task: returns -1 when queue full");

  // ─── Attention bank ───────────────────────────────────────────────────────
  SECTION("dtesn_sched: attention bank");

  dtesn_sched_reset();

  int atom1 = 0, atom2 = 0;

  // Insert new atoms.
  dtesn_sched_update_attention(&atom1, 10.0);
  dtesn_sched_update_attention(&atom2, 5.0);

  double sti1 = dtesn_sched_get_attention(&atom1);
  double sti2 = dtesn_sched_get_attention(&atom2);

  if (sti1 < 10.0 - TOLERANCE || sti1 > 10.0 + TOLERANCE) {
    fprintf(stderr, "  atom1 STI expected ~10.0, got %f\n", sti1);
    FAIL("update_attention: atom1 STI incorrect");
  }
  PASS("dtesn_sched_update_attention: atom1 STI = 10.0");

  if (sti2 < 5.0 - TOLERANCE || sti2 > 5.0 + TOLERANCE) {
    fprintf(stderr, "  atom2 STI expected ~5.0, got %f\n", sti2);
    FAIL("update_attention: atom2 STI incorrect");
  }
  PASS("dtesn_sched_update_attention: atom2 STI = 5.0");

  // Accumulate more STI.
  dtesn_sched_update_attention(&atom1, 2.5);
  sti1 = dtesn_sched_get_attention(&atom1);
  if (sti1 < 12.5 - TOLERANCE || sti1 > 12.5 + TOLERANCE) {
    fprintf(stderr, "  atom1 STI expected ~12.5 after delta, got %f\n", sti1);
    FAIL("update_attention: atom1 STI accumulation incorrect");
  }
  PASS("dtesn_sched_update_attention: atom1 STI accumulated to 12.5");

  // Decay: 0.5 factor should halve all STIs.
  dtesn_sched_decay_attention(0.5);
  sti1 = dtesn_sched_get_attention(&atom1);
  sti2 = dtesn_sched_get_attention(&atom2);

  if (sti1 < 6.25 - TOLERANCE || sti1 > 6.25 + TOLERANCE) {
    fprintf(stderr, "  atom1 STI expected ~6.25 after decay, got %f\n", sti1);
    FAIL("decay_attention: atom1 STI not halved");
  }
  PASS("dtesn_sched_decay_attention: atom1 STI halved to ~6.25");

  if (sti2 < 2.5 - TOLERANCE || sti2 > 2.5 + TOLERANCE) {
    fprintf(stderr, "  atom2 STI expected ~2.5 after decay, got %f\n", sti2);
    FAIL("decay_attention: atom2 STI not halved");
  }
  PASS("dtesn_sched_decay_attention: atom2 STI halved to ~2.5");

  // Unknown atom should return 0.
  int unknown = 0;
  if (dtesn_sched_get_attention(&unknown) != 0.0)
    FAIL("get_attention: unknown atom should return 0.0");
  PASS("dtesn_sched_get_attention: unknown atom returns 0.0");

  return 0;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(void) {
  printf("NodeCog Kernel Tests\n");
  printf("====================\n");

  struct ggml_context *ctx = nullptr;

  if (test_bootstrap(&ctx) != 0) return 1;
  if (test_hypergraph_fs(ctx) != 0) return 1;
  if (test_scheduler() != 0) return 1;

  printf("\n✓ All kernel tests passed!\n");
  return 0;
}
