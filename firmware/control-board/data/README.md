# Control-Board Audio Test Files

Place MP3 files for control-board event playback here, then upload SPIFFS.

Recognized filenames (first match wins by event):

- Startup: `startup.mp3`, `rocket_man.mp3`, `crocodile_rock.mp3`
- Attract loop: `attract.mp3`, `rocket_man.mp3`
- Game start: `start.mp3`, `crocodile_rock.mp3`
- Bonus: `bonus.mp3`, `crocodile_rock.mp3`
- Game over: `game_over.mp3`, `rocket_man.mp3`
- High score: `high_score.mp3`, `rocket_man.mp3`

Upload steps from `firmware/control-board/`:

```powershell
pio run -e captain_control_bringup -t buildfs
pio run -e captain_control_bringup -t uploadfs
```

Current firmware behavior:

- On boot, firmware tries Startup MP3 once through MAX98357 I2S.
- You can trigger event clips from serial monitor with commands:
	- `mp3 help`
	- `mp3 startup`
	- `mp3 attract` (loops)
	- `mp3 start`
	- `mp3 bonus`
	- `mp3 gameover`
	- `mp3 hiscore`
	- `mp3 stop`
- If no suitable MP3 file exists for startup, firmware falls back to the built-in startup tone test.