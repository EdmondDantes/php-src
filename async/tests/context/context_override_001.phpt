--TEST--
Context: override current context
--FILE--
<?php

Async\Context::current(true)->setKey('test-key', 'test-value');

Async\run(function() {
    Async\Context::overrideCurrent()->setKey('test-key-2', 'test-value-2');

    Async\run(function() {
        $value = Async\Context::current()->get('test-key').' : '.Async\Context::current()->get('test-key-2');
        echo "async function 3: {$value}\n";
    });
});

Async\run(function() {
    $value = Async\Context::current()->get('test-key');
    echo "async function 2: {$value}\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 2: test-value
async function 3: test-value : test-value-2
end
