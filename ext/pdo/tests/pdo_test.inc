<?php
# PDO test framework utilities

if (getenv('PDOTEST_DSN') === false) {
    $common = '';
    $append = false;
    foreach(file(dirname($_SERVER['PHP_SELF']).'/common.phpt') as $line) {
        if ($append) {
            $common .= $line;
        } elseif (trim($line) == '--REDIRECTTEST--') {
            $append = true;
        }
    }
    $conf = eval($common);
    foreach($conf['ENV'] as $n=>$v) putenv("$n=$v");
}

class PDOTest {
    // create an instance of the PDO driver, based on
    // the current environment
    static function factory($classname = PDO::class, bool $useConnectMethod = false) {
        $dsn = getenv('PDOTEST_DSN');
        $user = getenv('PDOTEST_USER');
        $pass = getenv('PDOTEST_PASS');
        $attr = getenv('PDOTEST_ATTR');
        if (is_string($attr) && strlen($attr)) {
            $attr = unserialize($attr);
        } else {
            $attr = null;
        }

        if ($user === false) $user = NULL;
        if ($pass === false) $pass = NULL;

        if ($useConnectMethod) {
            $db = $classname::connect($dsn, $user, $pass, $attr);
        } else {
            $db = new $classname($dsn, $user, $pass, $attr);
        }

        if (!$db) {
            die("Could not create PDO object (DSN=$dsn, user=$user)\n");
        }
        // Ignore errors about non-existent tables
        $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT);

        $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);
        $db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
        $db->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, true);
        return $db;
    }

    static function skip() {
        try {
            $db = PDOTest::factory();
        } catch (PDOException $e) {
            die("skip " . $e->getMessage());
        }
    }

    static function test_factory($file, $classname = PDO::class, bool $useConnectMethod = false) {
        $config = self::get_config($file);
        foreach ($config['ENV'] as $k => $v) {
            putenv("$k=$v");
        }
        return self::factory($classname, $useConnectMethod);
    }

    static function get_config($file) {
        $data = file_get_contents($file);
        $data = preg_replace('/^.*--REDIRECTTEST--/s', '', $data);
        $config = eval($data);

        return $config;
    }

    public static function dropTableIfExists(PDO $db, string $tableName): void {
        if (getenv('PDOTEST_DSN') === "sqlite::memory:") {
            return;
        }

        match ($db->getAttribute(PDO::ATTR_DRIVER_NAME)) {
            "oci" => $db->exec(<<<SQL
                 BEGIN
                     EXECUTE IMMEDIATE 'DROP TABLE $tableName';
                 EXCEPTION
                     WHEN OTHERS THEN
                         IF SQLCODE != -942 THEN
                             RAISE;
                         END IF;
                    END;
                SQL
            ),
            "firebird" => @$db->exec("DROP TABLE $tableName"),
            default => $db->exec("DROP TABLE IF EXISTS $tableName"),
        };
    }
}
/** See https://stackoverflow.com/a/3732466 */
function get_dummy_sql_request(): string
{
    $dsn = getenv('PDOTEST_DSN');

    // Firebird: https://www.firebirdfaq.org/faq30/
    if (str_starts_with($dsn, 'firebird')) {
        return "SELECT FIRST 0 FROM (VALUES ('Hello world')";
    }

    // Oracle
    if (str_starts_with($dsn, 'oci')) {
        return 'SELECT 1 FROM DUAL';
    }

    return 'SELECT 1';
}
