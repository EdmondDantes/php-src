--TEST--
Channel: consumer break data receiving cycle by closing the channel
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {

        while (true) {
            try {
                $data = $channel->receive();
                echo "receive by 1: $data\n";
            } catch (Async\ChannelWasClosed $e) {
                echo "channel closed\n";
                break;
            }
        }
    });

    Async\run(function() use($channel) {

        while (true) {
            try {
                $data = $channel->receive();
                echo "receive by 2: $data\n";
            } catch (Async\ChannelWasClosed $e) {
                echo "channel closed\n";
                break;
            }
        }
    });

    for ($i = 0; $i < 5; $i++) {
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
producer catch 'channel closed'
end
