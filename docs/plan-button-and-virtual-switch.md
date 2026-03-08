# Plan: Physical Button, Virtual Switch API, and Status UI

This plan adds three capabilities to the auto-DJ system:

1. **Physical button** on the Arduino as a second activation source (alongside the relay)
2. **Virtual switch API** on the orchestrator for dj-site to activate/deactivate auto-DJ and query status
3. **Status indicator** in the dj-site UI showing auto-DJ state

## Context

Today the Arduino is entirely relay-driven: the mixing board's AUX relay determines whether auto-DJ is active. There is no manual override and no remote visibility into the auto-DJ state from the DJ's perspective. The orchestrator repo exists but has no implementation -- only a README linking to the networking spec in this repo.

The networking spec (sections 2.4, 2.6, 3.8) defines management server endpoints for device administration (`/api/auto-dj/status`, `/api/auto-dj/commands`), but does not define a **virtual switch** API for activation/deactivation from dj-site. The orchestrator README references sections (2.2 "Virtual Switch Activation", 3.4 "Auto-DJ activation API") that do not yet exist in the networking spec.

## Scope

### In scope

- Spec the virtual switch API endpoints on the orchestrator (activate, deactivate, status)
- Spec the dj-site UI component for auto-DJ status
- Add physical button to the Arduino wiring spec
- Update the networking spec with the missing virtual switch sections
- Update wiring.md, config.h docs, and CLAUDE.md

### Out of scope

- Orchestrator implementation (no code exists yet; spec only)
- Backend-Service schema changes (covered by the existing `is_automation` plan in networking-spec section 5)
- Arduino firmware changes (button input, state machine modifications)
- dj-site implementation

## 1. Virtual Switch API (Orchestrator)

The orchestrator exposes these endpoints for dj-site (and any future admin UI) to control auto-DJ activation. These are separate from the device management endpoints in networking-spec section 3.8, which are Arduino-facing.

### Endpoints

#### `POST /api/auto-dj/activate`

Activates the auto-DJ system. The orchestrator starts a show on the configured flowsheet backend(s) and begins subscribing to AzuraCast for track changes.

**Auth**: Better Auth session/JWT. Requires `dj` role or higher.

**Request body**: None required. The orchestrator uses its own configured identity (Auto DJ DJ record).

**Response** (200):
```json
{
  "active": true,
  "activatedBy": {
    "source": "virtual_switch",
    "userId": "usr_abc123",
    "userName": "DJ Moonbeam"
  },
  "activatedAt": "2026-03-07T22:15:00Z",
  "showId": 789,
  "currentTrack": null
}
```

**Error responses**:
- `409 Conflict`: Auto-DJ is already active. Response includes the current status.
- `409 Conflict`: A live DJ show is in progress. Auto-DJ cannot activate while a DJ is broadcasting.
- `403 Forbidden`: Insufficient permissions.

**Side effects**:
- Orchestrator calls `POST /flowsheet/join` on Backend-Service (and/or `startRadioShow` on tubafrenzy) to create a show.
- Orchestrator begins subscribing to AzuraCast now-playing feed.
- If the Arduino is connected via the management channel, the orchestrator sends it a `resume` command (in case it was paused).

#### `POST /api/auto-dj/deactivate`

Deactivates the auto-DJ system. The orchestrator ends the current show and stops writing to the flowsheet.

**Auth**: Better Auth session/JWT. Requires `dj` role or higher.

**Request body**: None required.

**Response** (200):
```json
{
  "active": false,
  "deactivatedBy": {
    "source": "virtual_switch",
    "userId": "usr_abc123",
    "userName": "DJ Moonbeam"
  },
  "deactivatedAt": "2026-03-07T23:45:00Z"
}
```

**Error responses**:
- `409 Conflict`: Auto-DJ is not currently active.
- `403 Forbidden`: Insufficient permissions.

**Side effects**:
- Orchestrator calls `POST /flowsheet/end` on Backend-Service (and/or `finishRadioShow` on tubafrenzy) to end the show.
- Orchestrator stops subscribing to AzuraCast.
- If the Arduino is connected, the orchestrator sends it a `pause` command.

#### `GET /api/auto-dj/status`

Returns the current auto-DJ status. Available to any authenticated user (read-only).

**Auth**: Better Auth session/JWT. Requires `dj` role or higher for full status; unauthenticated users receive a minimal response (active/inactive only).

**Response** (200):
```json
{
  "active": true,
  "activatedBy": {
    "source": "virtual_switch",
    "userId": "usr_abc123",
    "userName": "DJ Moonbeam"
  },
  "activatedAt": "2026-03-07T22:15:00Z",
  "showId": 789,
  "currentTrack": {
    "artist": "Juana Molina",
    "title": "la paradoja",
    "album": "DOGA",
    "detectedAt": "2026-03-07T23:42:18Z"
  },
  "device": {
    "online": true,
    "transport": "ethernet",
    "lastHeartbeat": "2026-03-07T23:44:30Z",
    "relayState": "auto_dj_active"
  }
}
```

When auto-DJ is inactive:
```json
{
  "active": false,
  "lastDeactivatedAt": "2026-03-07T20:00:00Z",
  "lastDeactivatedBy": {
    "source": "relay",
    "detail": "Live DJ detected"
  },
  "device": {
    "online": true,
    "transport": "ethernet",
    "lastHeartbeat": "2026-03-07T23:44:30Z",
    "relayState": "dj_live"
  }
}
```

The `device` block is null if the Arduino has never connected.

### Activation Sources

The orchestrator tracks three activation sources:

| Source | Trigger | How it reaches the orchestrator |
|--------|---------|-------------------------------|
| `virtual_switch` | DJ clicks activate/deactivate in dj-site | `POST /api/auto-dj/activate` or `/deactivate` |
| `button` | Physical mushroom button press on Arduino | Arduino sends a `toggle` message over the management channel (WebSocket or HTTP) |
| `relay` | Mixing board AUX relay state change | Arduino reports relay state in heartbeat; orchestrator auto-deactivates when relay indicates a live DJ is broadcasting |

### Conflict Resolution

When multiple sources interact:

1. **Live DJ always wins.** If the relay reports a live DJ (`is_live: true` from AzuraCast, or relay open), the orchestrator deactivates auto-DJ regardless of the virtual switch state. The status response shows `deactivatedBy.source: "relay"`.

2. **Button and virtual switch are equivalent.** Both toggle the orchestrator's activation state. The last action wins. There is no precedence between them.

3. **Reactivation after live DJ.** When the relay transitions back to auto-DJ-active (live DJ signs off), the orchestrator does NOT automatically reactivate. A DJ must explicitly activate via the virtual switch or the physical button. This prevents the auto-DJ from unexpectedly resuming after a DJ finishes their show.

### Button Toggle via Management Channel

The Arduino sends a new message type over the management channel when the physical button is pressed:

```json
{
  "type": "button_toggle",
  "timestamp": 1709852100
}
```

The orchestrator responds by activating or deactivating (toggling the current state). The ack uses the standard `AutoDJAck` schema with an optional `result` field:

```json
{
  "type": "ack",
  "id": "btn_1709852100",
  "status": "ok",
  "result": { "active": true }
}
```

The `result` field is an optional extension to the existing `AutoDJAck` schema (which has `type`, `id`, `status`, and optional `error`). Adding an optional `result: object` field is backward-compatible -- existing ack consumers ignore unknown fields. The Arduino uses `result.active` to update its local knowledge of the orchestrator's state (for the status LED), but does not depend on it for state machine transitions.

Over WiFi fallback, the button press is reported in the next heartbeat:

```json
{
  "type": "heartbeat",
  "button_press_count": 1,
  ...
}
```

The `button_press_count` field is the number of button presses since the last heartbeat. The Arduino increments a counter on each debounced press and resets it to 0 after the heartbeat is sent. The orchestrator toggles state only if the count is odd (even presses cancel out). This eliminates the edge case where multiple presses between heartbeats produce the wrong result -- a boolean flag could not distinguish 1 press from 2.

## 2. dj-site UI Component

### Location

The auto-DJ status indicator lives in the **dashboard layout header**, visible on all authenticated pages. It should not be in the Appbar (which is a small fixed-position element for version/theme), but rather in the main content header area where DJs already see context about the current show.

### Component: `AutoDJStatusBanner`

A banner component that appears at the top of the dashboard when auto-DJ is active, and shows a compact indicator when inactive.

**Active state:**
- Background: amber/warning tone (MUI Joy `warning` color)
- Content: "Auto DJ is active" with current track info (artist - title) and a "Deactivate" button
- The "Deactivate" button is visible to DJs with `dj` role or higher

**Inactive state:**
- Compact: a small "Auto DJ: Off" indicator, or hidden entirely if the DJ is in the middle of their own show (no point showing auto-DJ status while they're live)
- An "Activate" button is available for DJs when no live show is in progress

**Device offline:**
- If `device.online` is false, show a subtle warning icon with tooltip: "Auto DJ hardware is offline"

### RTK Query Integration

New API slice at `lib/features/autoDJ/api.ts`:

```typescript
// The orchestrator is a separate service from Backend-Service.
// A new base query function is needed with its own env var.
const orchestratorBaseQuery = fetchBaseQuery({
  baseUrl: `${process.env.NEXT_PUBLIC_ORCHESTRATOR_URL}/api/auto-dj`,
  prepareHeaders: async (headers) => {
    const token = await getJWTToken();
    if (token) headers.set("Authorization", `Bearer ${token}`);
    return headers;
  },
});

export const autoDJApi = createApi({
  reducerPath: "autoDJApi",
  baseQuery: orchestratorBaseQuery,
  tagTypes: ["AutoDJStatus"],
  endpoints: (builder) => ({
    getStatus: builder.query<AutoDJStatus, void>({
      query: () => "/status",
      providesTags: ["AutoDJStatus"],
      // Poll every 10 seconds for near-real-time status
      pollingInterval: 10_000,
    }),
    activate: builder.mutation<AutoDJActivateResponse, void>({
      query: () => ({ url: "/activate", method: "POST" }),
      invalidatesTags: ["AutoDJStatus"],
    }),
    deactivate: builder.mutation<AutoDJDeactivateResponse, void>({
      query: () => ({ url: "/deactivate", method: "POST" }),
      invalidatesTags: ["AutoDJStatus"],
    }),
  }),
});
```

**Note on service routing**: The orchestrator is deployed as a standalone service on Railway, separate from Backend-Service. dj-site needs a `NEXT_PUBLIC_ORCHESTRATOR_URL` environment variable pointing to the orchestrator's URL. The `orchestratorBaseQuery` reuses the same JWT auth as `backendBaseQuery` (Better Auth tokens are valid across services that share the same JWKS endpoint), but routes to a different host.

### Types

New type file at `lib/features/autoDJ/types.ts`, mirroring the API response shapes defined in section 1. These types should ultimately come from `@wxyc/shared/auto-dj` (generated from `api.yaml`), but can be defined locally in dj-site until the shared package is updated.

### Polling vs. WebSocket

The initial implementation uses RTK Query polling (10-second interval). This is simple and sufficient -- auto-DJ status changes are infrequent (minutes to hours between transitions). A future optimization could use Server-Sent Events from the Backend-Service `/events` endpoint to push status changes, but polling is the right starting point.

## 3. Arduino Button Input

### Hardware

- **Button**: Industrial 22mm mushroom head, momentary (spring return), 1NO+1NC contacts, in a control station enclosure box
- **Pin**: D5 (`BUTTON_PIN`), configured as `INPUT_PULLUP`
- **Wiring**: Button NO contact to D5, common terminal to GND
- **Debounce**: 50ms (same as relay), handled by a new `ButtonMonitor` class following the `RelayMonitor` pattern

D5 is chosen because D2 (relay) and D3 (status LED) are taken, D4 is the Ethernet Shield Rev2's SD card chip select (even if unused, the shield's PCB connects D4 to the SD card slot -- using it for the button would create a latent conflict), and D10-D13 are reserved for the Ethernet Shield SPI bus (Phase 2). D5-D9 are all free.

### State Machine Changes

The button does NOT directly control the state machine. Instead:

1. `ButtonMonitor` detects a debounced press (rising edge on release, or falling edge on press -- TBD based on the specific button's NO wiring)
2. The main loop sends a `button_toggle` message to the orchestrator via the management channel
3. The orchestrator decides whether to activate or deactivate
4. The orchestrator's response flows back as a command, which the state machine already handles

This keeps the state machine pure and the Arduino "dumb" -- all activation logic lives in the orchestrator.

### Fallback: No Orchestrator

Before the orchestrator is implemented, the button has no effect. The Arduino continues to operate in relay-only mode. The button input is wired and debounced, but the `button_toggle` message has nowhere to go. This is fine -- the button hardware can be installed ahead of the orchestrator software.

## 4. Document Updates

### `docs/wiring.md`

Add button pin assignment:

| Pin | Function | Mode | Wiring |
|-----|----------|------|--------|
| D5 | Manual toggle button | `INPUT_PULLUP` | Button NO terminal -> D5, common terminal -> GND |

Update the wiring diagram to include the button.

### `docs/networking-spec.md`

Add new sections (do NOT renumber existing sections -- that would break cross-references throughout the document):

- **New section 2.7 "Activation Sources"**: Covers relay, button, and virtual switch as activation sources, with the conflict resolution rules from this plan
- **New section 3.10 "Virtual Switch API"**: The orchestrator-facing endpoints defined in this plan (activate, deactivate, status). Note: these are not Arduino-facing protocols (unlike the rest of section 3), but they are included here because the networking spec is the single source of truth for all auto-DJ network traffic and the orchestrator README explicitly references it
- **Section 3.6.2**: Add `button_toggle` to the WebSocket message types table
- **Section 3.7**: Add `button_press_count` field to the HTTP heartbeat schema
- **Section 5.2**: Add `AutoDJButtonToggle`, `AutoDJActivateResponse`, `AutoDJDeactivateResponse`, `AutoDJStatus` schemas to the `api.yaml` additions. Add `AutoDJButtonToggle` to the `AutoDJWebSocketMessage` `oneOf` discriminated union. Extend `AutoDJAck` with optional `result: object` field
- **Section 8**: Add open question about button debounce edge (press vs. release)

Also reconcile a pre-existing discrepancy: the `AutoDJHeartbeat` schema in section 5.2.2 lists state values `BOOTING, IDLE, STARTING_SHOW, AUTO_DJ_ACTIVE, ENDING_SHOW`, but the actual `State` enum in `state_machine.h` includes `CONNECTING_WIFI` and `ERROR_STATE` as well. Fix the schema to include all seven states.

### `docs/remote-administration.md`

Add to pin assignments table:

| Parameter | Current value | Purpose |
|-----------|--------------|---------|
| `BUTTON_PIN` | `5` | Manual toggle button (INPUT_PULLUP) |

### `CLAUDE.md` (this repo)

Add button and virtual switch to the key files table and development notes.

### Orchestrator `README.md`

Update the architecture diagram to include the physical button path and the virtual switch API endpoints. Fix the section references table to point to the actual networking spec sections. The current README references sections that do not exist (2.2 "Virtual Switch Activation", 2.3 "Conflict Resolution", 3.4 "Auto-DJ activation API"). The corrected mapping:

| Orchestrator README currently says | Should reference |
|---|---|
| 2.2 Virtual Switch Activation | 2.7 Activation Sources (new) |
| 2.3 Conflict Resolution | 2.7 Activation Sources (new, subsection on conflict resolution) |
| 2.6 Dual-Database Architecture | 2.3 Dual-Backend Architecture |
| 3.4 Auto-DJ activation API | 3.10 Virtual Switch API (new) |
| 3.5 Flowsheet write pipeline | 3.3-3.4 (tubafrenzy + Backend-Service flowsheet operations) |
| 3.8-3.9 Arduino management channel | 3.6-3.8 (WebSocket management + HTTP fallback + server-side endpoints) |

## 5. Sequencing

This plan produces spec documents only. Implementation is tracked separately.

| Deliverable | Depends on |
|------------|-----------|
| Virtual switch API spec (networking-spec.md additions) | This plan |
| dj-site UI spec (component + RTK Query design) | This plan |
| `api.yaml` schema additions for virtual switch types | Virtual switch API spec |
| Arduino button wiring spec (wiring.md, config.h) | This plan |
| Orchestrator README updates | Virtual switch API spec |

The virtual switch API spec is the foundation. The dj-site UI and Arduino button specs can proceed in parallel once the API shape is defined.
