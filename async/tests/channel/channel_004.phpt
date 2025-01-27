--TEST--
Channel: consumer break data receiving cycle by closing the channel
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {

        $receiveLimit = 3;
        $received = 0;

        while (($data = $channel->receive()) != null) {
            echo "receive: $data\n";
            $received++;
            if ($received >= $receiveLimit) {
                $channel->close();
                break;
            }
        }
    });

    for ($i = 0; $i < 4; $i++) {
        try {
            $channel->send("event data $i");
        } catch (Async\ChannelWasClosed $e) {
            echo "producer catch 'channel closed'\n";
            break;
        }
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
