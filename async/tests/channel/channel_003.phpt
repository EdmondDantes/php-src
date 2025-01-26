--TEST--
Channel: send and receive in cycle
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while (true) {
            try {
                $data = $channel->receive();
                echo "receive: $data\n";
            } catch (Async\ChannelWasClosed $e) {
                echo "channel closed\n";
                break;
            } catch (Async\ChannelProducingFinished $e) {
                echo "producing finished\n";
                break;
            }
        }
    });

    for ($i = 0; $i < 4; $i++) {
        $channel->send("event data $i");
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive: event data 0
receive: event data 1
receive: event data 2
receive: event data 3
channel closed
end
