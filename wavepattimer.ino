/*
 * wave-pat-from-timer
 * Publish and respond to wave/pat events based on a timer.
 * subscribe to these (fake) events and treat digital events as a remote 'wave'
 * (flash the LED 3 times HIGH/LOW in response), treat analog events as a 'pat'
 * (flash the LED 3 times at the given intensity).
 */
SerialLogHandler logHandler;

int led0 = D7; /* reserved, wire an LED to this one to see it blink */

const int blinktime = 1000;   /* millisecond LED blink duration */
int give_us_a_wave = 0;
int give_us_a_pat = 0;

void wave_every_five()
{
    static int counter;
    int mixer = random(3);

    if (counter++ % mixer)
        give_us_a_pat = 1;
    else
        give_us_a_wave = 1;
}
Timer timer(5000, wave_every_five);

int wave()
{
    /* blink the LED 3 times */
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(led0, HIGH);
        delay(blinktime / 3);
        digitalWrite(led0, LOW);
        delay(blinktime / 3);
    }
    return 0;
}

/* Respond to a digital 'wave' event */
int wave_event(const char *event, const char *data)
{
    static int i;
    Log.info("%d: event=%s data=%s", ++i, event, (data ? data : "NULL"));

    return wave();
}

int pat(int value)
{
    for (int i = 0; i < 3; i++)
    {
        analogWrite(led0, value);
        delay(blinktime / 3);
        analogWrite(led0, 0);
        delay(blinktime / 3);
    }
    return 0;
}

/* Respond to an analog 'pat' event */
int pat_event(const char *event, const char *data)
{
    /* blink the LED 3 times at given intensity */
    static int i;
    Log.info("%d: event=%s data=%s", ++i, event, (data ? data : "NULL"));

    int value = data ? atoi(data) : 64;
    return pat(value);
}

void setup()
{
    /* use the reserved pin for output LED */
    pinMode(led0, OUTPUT);

    /* handlers for the fake 'wave' and 'pat' events */
    Particle.subscribe("wave", wave_event);
    Particle.subscribe("pat", pat_event);

    /* start the event generator */
    timer.start();
}

/*
 * Handle event publishing in the main loop, not in the timer callback -
 * docs.particle.io recommends not publishing events in timer interrupt.
 */
void loop()
{
    if (give_us_a_wave)
    {
        give_us_a_wave = 0;
        Log.info("Publishing wave now");
        Particle.publish("wave", PUBLIC);
    }

    if (give_us_a_pat)
    {
        give_us_a_pat = 0;
        int ledValue = random(64);
        String value = String(ledValue);
        Log.info("Publishing pat now: %d", ledValue);
        Particle.publish("pat", value, PUBLIC);
    }
}
