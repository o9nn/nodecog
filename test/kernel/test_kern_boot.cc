// Test kernel boot functions
#include "src/kern_boot.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main() {
  printf("Testing kernel bootstrap functions...\n");
  
  // Test Stage 0: Bootstrap
  struct ggml_context* ctx = nullptr;
  int result = kern_boot_stage0(&ctx, 128 * 1024 * 1024);  // 128MB
  
  if (result != 0) {
    fprintf(stderr, "Stage 0 failed with error code: %d\n", result);
    return 1;
  }
  
  if (ctx == nullptr) {
    fprintf(stderr, "Stage 0 returned null context\n");
    return 1;
  }
  
  printf("✓ Stage 0: Bootstrap completed successfully\n");
  
  // Test Stage 1: Hypergraph Filesystem
  struct hgfs_config hgfs_cfg = {
    .max_depth = 16,
    .initial_atom_capacity = 10000,
    .enable_attention_gc = true
  };
  
  result = kern_boot_stage1_init_hypergraph_fs(ctx, &hgfs_cfg);
  
  if (result != 0) {
    fprintf(stderr, "Stage 1 failed with error code: %d\n", result);
    return 1;
  }
  
  printf("✓ Stage 1: Hypergraph Filesystem initialized\n");
  
  // Test Stage 2: Scheduler
  struct dtesn_config sched_cfg = {
    .reservoir_size = 1000,
    .tick_rate_us = 5,
    .sti_threshold = 0.5,
    .lti_threshold = 0.3,
    .enable_adaptive_scheduling = true
  };
  
  struct dtesn_sched_context* sched = nullptr;
  result = kern_boot_stage2_init_scheduler(&sched, &sched_cfg);
  
  if (result != 0) {
    fprintf(stderr, "Stage 2 failed with error code: %d\n", result);
    return 1;
  }
  
  if (sched == nullptr) {
    fprintf(stderr, "Stage 2 returned null scheduler\n");
    return 1;
  }
  
  printf("✓ Stage 2: DTESN Scheduler initialized\n");
  
  // Test version info
  uint16_t major, minor, patch;
  kern_get_version(&major, &minor, &patch);
  printf("✓ Kernel version: %d.%d.%d\n", major, minor, patch);
  
  // Cleanup
  dtesn_sched_destroy(sched);
  
  printf("\n✓ All kernel bootstrap tests passed!\n");
  return 0;
}
