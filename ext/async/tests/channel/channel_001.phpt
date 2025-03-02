--TEST--
Channel: basic test
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        echo "async function 2\n";

        $data = $channel->receive();
        echo "receive: $data\n";
    });

    $channel->send("event data");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 2
receive: event data
end
