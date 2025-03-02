--TEST--
Async\run() with multiple parameters and multiple calls
--FILE--
<?php

$function = function(string $parameter1, array $parameter2, int $parameter3) {
    echo "async function: $parameter1 $parameter3\n";
    foreach ($parameter2 as $key => $value) {
        echo "$key: $value\n";
    }
};

$parameter1 = "1";
$parameter2 = ["key" => "value"];
$parameter3 = 2;

Async\run($function, $parameter1, $parameter2, $parameter3);
Async\run($function, "2", $parameter2, $parameter3);

unset($function, $parameter1, $parameter2, $parameter3);

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function: 1 2
key: value
async function: 2 2
key: value
end
