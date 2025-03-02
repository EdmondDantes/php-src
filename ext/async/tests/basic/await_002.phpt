--TEST--
Exception captured in awaiting async function
--FILE--
<?php

Async\run(function() {
    echo "async function 1\n";

    try {
        Async\await(function() {
            echo "async function 2\n";
            throw new Exception("function2 error");
        });
    } catch (Exception $e) {
        echo "Exception '{$e->getMessage()}' captured in async function 1\n";
    }
});

Async\run(function() {
    echo "async function 3\n";
});


echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
async function 3
async function 2
Exception 'function2 error' captured in async function 1
end
