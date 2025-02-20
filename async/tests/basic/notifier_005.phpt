--TEST--
Notifier removeCallback method
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
    $callback = new Async\Callback(function(Async\Notifier $notifier, mixed $data) {
                        echo "callback from {$notifier->toString()}: $data\n";
                    });

    $notifier->addCallback($callback);
    $notifier->removeCallback($callback);
    $notifier->trigger();

    echo "async function 1\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
end
