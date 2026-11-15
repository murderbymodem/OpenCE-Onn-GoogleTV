/*
DSOUND_ONN_GOOGLETV.INL

OpenCE Onn GoogleTV's change to the mixer's lock (dsound_sdl.c includes it
just after mixer_lock): kept in a file of the fork's own, so that the main
project's edits of dsound_sdl.c merge with it. Its calls of
pthread_mutex_lock(&mixer_lock) take the lock through the gate below (the
macro at the end); the mix itself takes it with mixer_take_to_mix.
*/

/* the most frames the device's callback mixes in one turn with the lock
(mixer_gate): the 32-bit Android app's a few milliseconds' worth, so that
the game's thread never waits long for it; a voice's ramps take the same
frames however many a mix has (mix_voice) */
#ifdef HALO_ARM32_GUEST
#define MIX_TURN_FRAMES 128
#else
#define MIX_TURN_FRAMES MIX_CHUNK_FRAMES
#endif
/* The mix holds mixer_lock while it mixes every voice, and takes it again
as soon as it has let it go; a mutex is not handed to whoever waits, so the
game's thread, which takes the lock for every sound it starts, moves or
stops, could wait through mix after mix. On a 32-bit Android TV box, where a
busy scene's mix takes 50 to 90% of the time it plays for, that was a frame
late whenever many sounds played. So everything but the mix takes the lock
through mixer_gate, and the mix passes through the gate before each turn: it
then waits there until whoever is at the gate has had the lock. The mix's
turns are also short there (MIX_TURN_FRAMES).

The game's thread takes the lock in bursts: a frame's sounds, some dozens of
calls one after another. Were the mix to take its next turn after the first
of them, each call of the burst would wait a turn out, a burst of thirty
some 70 ms. So where the turns are short the mix also stays out while a
burst lasts: if the lock was taken through the gate since the mix's last
turn began, it waits until none has been for MIXER_BURST_QUIET_NS, a
MIXER_BURST_WAITS of them at most (under a millisecond, a third of a
turn's sound, so that the mix still keeps up if the calls never stop). */
static pthread_mutex_t mixer_gate = PTHREAD_MUTEX_INITIALIZER;
/* how many times the lock has been taken through the gate */
static unsigned long mixer_gate_passes;
#define MIXER_BURST_QUIET_NS 60000L
#define MIXER_BURST_WAITS 8

static void mixer_take(void)
{
	pthread_mutex_lock(&mixer_gate);
	pthread_mutex_lock(&mixer_lock);
	__atomic_add_fetch(&mixer_gate_passes, 1, __ATOMIC_RELAXED);
	pthread_mutex_unlock(&mixer_gate);
}

/* (the mix's: after anyone at the gate, and after a burst) */
static void mixer_take_to_mix(void)
{
#if MIX_TURN_FRAMES < MIX_CHUNK_FRAMES
	static unsigned long passes_at_last_turn;
	unsigned long passes = __atomic_load_n(&mixer_gate_passes, __ATOMIC_RELAXED);
	int waits;

	for (waits = 0; passes != passes_at_last_turn && waits < MIXER_BURST_WAITS; waits++)
	{
		struct timespec quiet = { 0, MIXER_BURST_QUIET_NS };

		passes_at_last_turn = passes;
		nanosleep(&quiet, NULL);
		passes = __atomic_load_n(&mixer_gate_passes, __ATOMIC_RELAXED);
	}
	passes_at_last_turn = passes;
#endif
	pthread_mutex_lock(&mixer_gate);
	pthread_mutex_unlock(&mixer_gate);
	pthread_mutex_lock(&mixer_lock);
}
/* every pthread_mutex_lock after this, in dsound_sdl.c: the mixer's lock
through the gate (mixer_take), any other as it is */
static int onn_googletv_mutex_lock(pthread_mutex_t *mutex)
{
	if (mutex == &mixer_lock)
	{
		mixer_take();
		return 0;
	}
	return pthread_mutex_lock(mutex);
}
#define pthread_mutex_lock(mutex) onn_googletv_mutex_lock(mutex)
