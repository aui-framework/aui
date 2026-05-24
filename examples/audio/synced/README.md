# Audio synced playback

<!-- aui:example audio -->
Demonstrates synchronized playback of multiple audio tracks with per-track volume and a simple visual gain meter.

- Choose a sample set with radio buttons; each set contains a base layer and additional layers.
- Control playback with Play/Pause and toggle Loop to automatically restart when tracks finish.
- Each track row shows:
    - A small color bar reflecting recent amplitude changes (greener = higher activity).
    - A volume slider (0–100%) applied in real time.

When any track finishes, the example resets all tracks to keep them aligned. If Loop is enabled, playback restarts
immediately.

## UI

- Playback controls: Play, Pause, Loop.
- Sample selector: radio group to switch between predefined layered samples.
- Tracks panel: rows for each layer with activity indicator and volume slider.

## Concepts

- Synchronized start/stop across multiple audio players.
- Reactive state for play/pause, loop, selection, and per-track volume.
- Lightweight visual gain analysis for a live activity indicator.