--TEST--
Notifier should not add the same callback twice
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
    $callback = new Async\Closure(function(Async\Notifier $notifier, mixed $data) {
                        echo "callback from {$notifier->getInfo()}: $data\n";
                    });

    $notifier->addCallback($callback);
    $notifier->addCallback($callback);
    $notifier->trigger();

    echo "async function 1\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
callback from CustomNotifier: event data
async function 1
end
