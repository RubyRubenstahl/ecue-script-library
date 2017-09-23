#Unified Control Protocol
The unfied control protocol emulates the RS232 control protocol of the Butler XT2, bringing the same command structure to be used in e:cue Programmer over UDP. This allows for a unified programming experience across devices. 

In addition to the commands supported by the Butler XT2, there are several additional commands providing access to features of programmer that are not available in the XT2 standalone mode. 



##Protocol Details
The protocol is fairly simple and contains only a few commands. All commands have the following aspecs in common: 
- Each command starts with a two-letter command code
- Each command has one or two 3-digit prameter with [leading 0's](https://en.wikipedia.org/wiki/Leading_zero)
- Each command should end with a carraige return and a newline ('\r\n')


|Command Code	|Parameters			|Action					|Example
|-----|-------------------|-----------------------|--------------
|PC		| 1 - Cuelist Index	| Play cuelist			| Play cuelist 5: `PC005\r\n`
|PQ		| 1 - Cuelist Index	<br> 2- Cue Number| Play a specific cue			| Play cuelist 3, Cue 2: `PC003002\r\n`
|TP		| 1 - Cuelist Index	| Toggle cuelist on/off | Toggle cuelist 3: `TP003\r\n`
|PP		| 1 - Cuelist Index | Toggle cuelist play/pause| Toggle cuelist 35 pause: `PP035\r\n`
|NX		| 1 - Mutex Group Index| Play the next cuelist in a *mutex group*<sup>1</sup>| Play next cue in *mutex group* 3: `NX003\r\n`
|PX		| 1 - Mutex Group Index| Play the previous cuelist in a *mutex group*<sup>1</sup>| Play previous cue in *mutex group* 75: `NX075\r\n`
|IN		| 1 - Fader Index <br>`0 = Grandmaster` <br>2 - Level (0-100) | Set the level of a fader. | Grandmaster to 50%: `IN000050\r\n` <br><br> Versatile master 3 to 100%: `IN003100\r\n`
|AF		| 1 - Fader Index <br>`0 = Grandmaster` <br><br>2 - Level (0-100) <br><br> 3 - Fade time| Slide a fader to a new value over a specified amount of time	| Grandmaster to 50% in 3 seconds: `IN000050003\r\n` <br><br>Versatile Master  5 to 80% in 2 seconds: `IN005080002\r\n`
|ST		| 1 - Cuelist Index	|Stop cuelist. <br><br>If the index is 0, all cuelists will be stopped. | Stop All Cuelists: `ST000\r\n`<br><br>Stop cuelist 22: `ST022\r\n`

<sup>[1]</sup> A mutexgroup is a group of cuelists from which only one cuelist can play at any given time. If one cuelist in the group is playing and another is started, the original cuelist is stopped automatically and the new cuelist plays.

