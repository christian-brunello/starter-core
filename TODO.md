# STARTER-core — TODO / checkpoint

Ultimo aggiornamento: 2026-09-14  
Branch: `async-client-start`  
Contesto: compat GLib/MySQL fatto; connect+snapshot client reso async; SetOutput ancora sync.

## 0. Housekeeping git

- [x] Commit lavoro su `async-client-start` (API async + mdns pending + test + fix mysql-client/Makefile + TODO)
- [ ] Decidere se `doc/starter-core.pdf` cancellato va committato o ripristinato (lasciato fuori dal commit async)
- [ ] Push del branch (opzionale; finora non richiesto)
- [ ] Verificare `make check` completo su una macchina “pulita” dopo il clone

## 1. Async D-Bus — ancora da fare

- [ ] **`st_client_set_output` async** (priorità alta per non bloccare il main loop nelle regole)
  - API `*_async` / `*_finish` o fire-and-forget + segnale errore
  - Coalescing per output (ultimo valore vince)
  - Update ottimistico della cache locale vs riconciliazione su `OutputChanged`
  - `GCancellable` legato al lifetime del client
  - Adattare `assign_apply` / `engine_apply` (oggi fallisce in sync se set_output fallisce)
- [ ] **`GetName` async o cache** al connect (oggi ancora `call_sync` in `st_client_get_service_name`)
- [ ] Timeout finiti al posto di `-1` su tutte le call D-Bus
- [ ] Stress async parallelo (variante B armageddon: N start in volo insieme) — opzionale

## 2. Bug / correttezza codice

- [ ] **`st_server_set_inputs` / `st_server_set_outputs`**: usano `g_object_unref` su `GPtrArray*` — dovrebbe essere `g_ptr_array_unref` / `g_ptr_array_ref` (bug serio se quei setter vengono usati; vedi `lib/server.c`)
- [ ] **`g_ptr_array_steal` in `lib/client.c`** (GetInputs/GetOutputs): return value ignorato → leak degli elementi vecchi a ogni refresh snapshot
- [ ] Leak potenziali di stringhe da `g_variant_get` dove non ancora `g_free` (verificare path sync/async oltre i signal handler già fixati)
- [ ] `st_client_set_output`: messaggio di log dice ancora "error call get name method"
- [ ] Typo storico: `lable_block_delete` vs `label_block_delete`

## 3. Sicurezza / produzione

- [ ] **Password MySQL in chiaro** in `src/schemas/org.starter.gschema.xml` (default hardcoded) — rimuovere default reale; usare secret/env/file 0600
- [ ] D-Bus server: `G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS` su TCP `0.0.0.0` — ok solo in LAN fidata; documentare o restringere bind/auth
- [ ] `starter-core.service.in`: `ExecStartPre=/bin/sleep 60` — fragile; sostituire con dipendenze systemd corrette / `Restart` + readiness
- [ ] Rules preprocessor (`m4 {F}` / comando da GSettings): rischio se il setting è controllabile — validare/escaping

## 4. Architettura / main loop

- [ ] MySQL ancora **sincrono** nelle callback I/O (`store_history`, label) — blocca il loop anche con D-Bus async
- [ ] Valutare worker thread / pool per DB, o API async MySQL
- [ ] `engine_apply` può rientrare via segnali durante call sync (e in futuro via OutputChanged dopo SetOutput async) — valutare idle queue / anti-ricorsione
- [ ] `engine_apply(..., NULL)` a ogni peer ready: carico se molti nodi si connettono insieme

## 5. Qualità / tooling

- [ ] `.gitignore` robusto per artefatti autotools/build (oggi tanti untracked: `Makefile`, `.o`, `.libs`, `valgrind.log`, swap vim, …)
- [ ] Aggiungere CI minima: `./autogen.sh && ./configure && make && make check`
- [ ] Allineare README (D-Bus peer TCP, non solo “D-Bus” generico) e versioni minime (GLib ≥ 2.44)

## 6. API / compatibilità

- [ ] Decidere se deprecare `st_client_start` sync o tenerlo come wrapper di comodo per test
- [ ] Documentare stati `IDLE/PENDING/READY/FAILED` in Texinfo
- [ ] `glib-compat`: monitorare layout `GRealPtrArray` se si supportano GLib < 2.62 a lungo

## 7. Test — gap

- [ ] Test esplicito su cancel durante pending + `service-removed` (path mdns)
- [ ] Test SetOutput (sync oggi; async quando arriverà)
- [ ] Test setter server inputs/outputs dopo fix refcounting
- [ ] Valgrind mirato su client start async + armageddon-async

## 8. Prossimi passi suggeriti (ordine)

1. Push opzionale di `async-client-start`
2. Fix `g_object_unref` su GPtrArray in server + leak `g_ptr_array_steal` in client
3. SetOutput async + adattamento engine
4. Hardening secrets/auth/systemd
5. MySQL off-main-loop
