**The Tic-Tac-Toe game in C that operates in the user space, while communicating with the kernel space through a Character Device File:**
_Step 1:_ Write I2C driver for ssd1306 screen 
_Step 2:_ Handle buttons using:
	+ gpio descriptor base (device tree)
	+ IRQ handling (gpio IRQ)
_Step 3:_ Write a TicTacToe game on userspace

**Video demo project**: https://www.youtube.com/watch?v=TrqDp2n_HKs
