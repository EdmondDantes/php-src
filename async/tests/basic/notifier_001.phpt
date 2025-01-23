--TEST--
Notifier basic test
--FILE--
<?php

class CustomNotifier extends Async\Notifier
{
    public function __construct(private string $data) {}

    public function trigger(): void
    {
        $this->notify($this->data);
    }
}

Async\async(function() {
    $notifier = new CustomNotifier("event data");
    $callback = new Async\Callback(function(mixed $data) {
                        echo "callback 1: $data\n";
                    });

    $notifier->addCallback($callback);

    echo "async function 1\n";

    $notifier->trigger();
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
callback 1: CustomNotifier
end
