--TEST--
Context: redefine key in async function
--FILE--
<?php

Async\Context::current(true)->setKey('test-key', 'test-value');

Async\run(function() {
    Async\Context::current()->setKey('test-key', 'test-value2', true);

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
async function 2: test-value2
async function 3: test-value2
end
