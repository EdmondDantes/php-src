--TEST--
Context: redefine context and key in async function
--FILE--
<?php

Async\Context::current(true)->setKey('test-key', 'test-value');

Async\run(function() {
    Async\Context::newCurrent()->setKey('test-key', 'test-value2');

    Async\run(function() {
        $value = Async\Context::current()->get('test-key');
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
async function 3: test-value2
end
