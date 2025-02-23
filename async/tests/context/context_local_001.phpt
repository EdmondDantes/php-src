--TEST--
Context: redefine key in async function
--FILE--
<?php

Async\Context::current(true)->setKey('test-key', 'test-value');

Async\run(function() {
    Async\Context::local()->setKey('test-key', 'test-value-local');
    $value = Async\Context::local()->get('test-key');

    echo "async function 1: {$value}\n";

    Async\run(function() {
        $value = Async\Context::local()->get('test-key');
        echo "async function 3: {$value}\n";
    });
});

Async\run(function() {
    $value = Async\Context::local()->get('test-key');
    echo "async function 2: {$value}\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1: test-value-local
async function 2: test-value
async function 3: test-value
end
