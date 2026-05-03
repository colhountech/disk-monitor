# disk-monitor

A lightweight native C program that monitors disk usage on physical filesystems with change tracking.

## Features

- Auto-detects physical mounted filesystems (excludes tmpfs, proc, etc.)
- Tracks percentage change since last run
- Human-readable sizes (like `df -h`)
- Simple append-only history log
- Clean, aligned output with trend arrows (▲ / ▼)
- Zero external dependencies
- Super Fast

## Example Output

```bash

🖥️  Disk Monitor — 2026-05-03

Root (/):     69% used ( 26.0G used,  11.3G free) ▲+2%
Data disk:    17% used ( 13.4G used,  64.8G free) → 0%

```

## Installation

make
sudo make install

Then view the manual with:

man disk-monitor

## Files

- /var/lib/disk-monitor/state.log — history log (one line per run)
- Binary installed to /usr/local/bin/disk-monitor

## Usage

Run manually:

disk-monitor

Add to cron (example — daily at 6 AM):

```bash
0 6 * * * /usr/local/bin/disk-monitor
```

## Building from Source

make
sudo make install

## Authors

Micheal Colhoun (github.com/colhountech/disk-monitor)
Co-Authored by Grok (built by xAI)

## License

MIT License © 2026 ColhounTech Limited
