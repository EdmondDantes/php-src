--TEST--
Channel: close + isClosed + isFull + isEmpty + isNotEmpty + isProducingFinished + getCapacity + getUsed
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while (($data = $channel->receiveOrNull()) != null) {
            echo "receive: $data\n";
        }
    });

    for ($i = 0; $i < 4; $i++) {
        $channel->send("event data $i");
    }

    $channel->close();

    echo "channel is closed: " . ($channel->isClosed() ? "yes" : "no") . "\n";
    echo "channel is full: " . ($channel->isFull() ? "yes" : "no") . "\n";
    echo "channel is empty: " . ($channel->isEmpty() ? "yes" : "no") . "\n";
    echo "channel is not empty: " . ($channel->isNotEmpty() ? "yes" : "no") . "\n";
    echo "channel is producing finished: " . ($channel->isProducingFinished() ? "yes" : "no") . "\n";
    echo "channel capacity: " . $channel->getCapacity() . "\n";
    echo "channel used: " . $channel->getUsed() . "\n";
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
channel is closed: yes
channel is full: no
channel is empty: yes
channel is not empty: no
channel is producing finished: yes
channel capacity: 8
channel used: 0
end
