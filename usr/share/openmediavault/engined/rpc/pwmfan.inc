<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2025 openmediavault plugin developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
namespace Engined\Rpc;

require_once("openmediavault/functions.inc");

class Pwmfan extends \OMV\Rpc\ServiceAbstract {
    private const SERVICE_NAME = "openmediavault-pwmfan";
    private const STATUS_FILE = "/run/openmediavault-pwmfan-status.json";

    public function getName() {
        return "Pwmfan";
    }

    public function initialize() {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getDashboard");
    }

    public function getSettings($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.pwmfan");
        return $object->getAssoc();
    }

    public function setSettings($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);
        $this->validateMethodParams($params, "rpc.pwmfan.setsettings");

        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.service.pwmfan");
        $object->setAssoc($params);
        $db->set($object);
        return $object->getAssoc();
    }

    public function getDashboard($params, $context) {
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $systemCtl = new \OMV\System\SystemCtl(self::SERVICE_NAME);
        $running = $systemCtl->isActive();
        $status = $this->readStatus();
        $tempValue = $status["temperature"] ?? null;
        $speedValue = $status["speed"] ?? null;

        return [
            [
                "title" => gettext("Service"),
                "value" => $running ? gettext("Running") : gettext("Stopped")
            ],
            [
                "title" => gettext("Temperature"),
                "value" => is_numeric($tempValue) ?
                    sprintf("%.1f C", $tempValue) :
                    gettext("n/a")
            ],
            [
                "title" => gettext("Fan Speed"),
                "value" => is_numeric($speedValue) ?
                    sprintf("%d%%", $speedValue) :
                    gettext("n/a")
            ]
        ];
    }

    private function readStatus() {
        if (!file_exists(self::STATUS_FILE)) {
            return null;
        }

        $content = file_get_contents(self::STATUS_FILE);
        if ($content === false) {
            return null;
        }

        $decoded = json_decode($content, true);
        if (!is_array($decoded)) {
            return null;
        }

        return $decoded;
    }
}
