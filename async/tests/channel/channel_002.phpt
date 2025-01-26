--TEST--
Channel: channel closed in cycle
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

    $channel->send("event data");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive: event data
channel closed
end
