# STARTER-core — TODO / checkpoint

Last updated: 2026-10-03  
Branch: `async-client-start`  
Context: GLib/MySQL compatibility done; client connect+snapshot is async; SetOutput still sync.

## 0. Git housekeeping

- [x] Commit work on `async-client-start` (async API + mdns pending + tests + mysql-client/Makefile fixes + TODO)
- [ ] Decide whether the deleted `doc/starter-core.pdf` should be committed as a removal or restored (left out of the async commit)
- [ ] Push the branch (optional; not requested so far)
- [ ] Run a full `make check` on a clean machine after clone

## 1. Async D-Bus — still to do

- [ ] **`st_client_set_output` async** (high priority: avoid blocking the main loop from rules)
  - `*_async` / `*_finish` API, or fire-and-forget + error signal
  - Per-output coalescing (last value wins)
  - Optimistic local cache update vs reconciliation on `OutputChanged`
  - `GCancellable` tied to the client lifetime
  - Adapt `assign_apply` / `engine_apply` (today sync failure of set_output fails the apply)
- [ ] **`GetName` async or cache** at connect (still `call_sync` in `st_client_get_service_name`)
- [ ] Finite timeouts instead of `-1` on all D-Bus calls
- [ ] Parallel async stress (armageddon variant B: N starts in flight together) — optional

## 2. Bugs / code correctness

- [ ] **`st_server_set_inputs` / `st_server_set_outputs`**: use `g_object_unref` on `GPtrArray*` — should be `g_ptr_array_unref` / `g_ptr_array_ref` (serious bug if those setters are used; see `lib/server.c`)
- [ ] **`g_ptr_array_steal` in `lib/client.c`** (GetInputs/GetOutputs): return value ignored → leak of old elements on every snapshot refresh
- [ ] Potential string leaks from `g_variant_get` where `g_free` is still missing (audit sync/async paths beyond the signal handlers already fixed)
- [ ] `st_client_set_output`: log message still says "error call get name method"
- [ ] Historical typo: `lable_block_delete` vs `label_block_delete`

## 3. Security / production

- [ ] **MySQL password in cleartext** in `src/schemas/org.starter.gschema.xml` (hardcoded default) — remove the real default; use a secret/env/0600 file
- [ ] D-Bus server: `G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS` on TCP `0.0.0.0` — acceptable only on a trusted LAN; document or tighten bind/auth
- [ ] `starter-core.service.in`: `ExecStartPre=/bin/sleep …` is fragile; replace with proper systemd dependencies / `Restart` + readiness
- [ ] Rules preprocessor (`m4 {F}` / GSettings command): risk if the setting is attacker-controlled — validate/escape

## 4. Architecture / main loop

- [ ] MySQL is still **synchronous** in I/O callbacks (`store_history`, labels) — blocks the loop even with async D-Bus
- [ ] Consider a worker thread/pool for DB, or an async MySQL API
- [ ] `engine_apply` can re-enter via signals during sync calls (and later via `OutputChanged` after async SetOutput) — consider an idle queue / anti-recursion
- [ ] `engine_apply(..., NULL)` on every peer ready: load spike if many nodes connect at once

## 5. Quality / tooling

- [ ] Robust `.gitignore` for autotools/build artifacts (today many untracked files: `Makefile`, `.o`, `.libs`, `valgrind.log`, vim swap, …)
- [ ] Add minimal CI: `./autogen.sh && ./configure && make && make check`
- [ ] Align README (D-Bus peer TCP, not just generic “D-Bus”) and minimum versions (GLib ≥ 2.44)

## 6. API / compatibility

- [ ] Decide whether to deprecate sync `st_client_start` or keep it as a convenience wrapper for tests
- [x] Document `IDLE` / `PENDING` / `READY` / `FAILED` states in Texinfo
- [ ] `glib-compat`: watch `GRealPtrArray` layout if GLib < 2.62 stays supported long-term

## 7. Tests — gaps

- [ ] Explicit test for cancel while pending + `service-removed` (mdns path)
- [ ] SetOutput tests (sync today; async when it lands)
- [ ] Server inputs/outputs setter tests after the refcounting fix
- [ ] Targeted Valgrind on async client start + armageddon-async

## 8. Suggested next steps (order)

1. Optional push of `async-client-start`
2. Fix `g_object_unref` on GPtrArray in server + `g_ptr_array_steal` leak in client
3. Async SetOutput + engine adaptation
4. Harden secrets / auth / systemd
5. Move MySQL off the main loop
