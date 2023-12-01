"Resource/UI/InGameOptionsFlyoutSurvival.res"
{
	"PnlBackground"
	{
		"ControlName"			"Panel"
		"fieldName"				"PnlBackground"
		"xpos"					"0"
		"ypos"					"0"
		"zpos"					"-1"
		"wide"					"156"
		"tall"					"85"
		"visible"				"1"
		"enabled"				"1"
		"paintbackground"		"1"
		"paintborder"			"1"
	}

	"BtnReturnToLobby"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnReturnToLobby"
		"xpos"					"0"
		"ypos"					"0"
		"wide"					"150"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnBootPlayer"
		"navDown"				"BtnRestartScenario"
		"tooltiptext"			"#L4D360UI_ReturnToLobby_Tip"
		"labelText"				"#L4D360UI_ReturnToLobby"
		"style"					"FlyoutMenuButton"
		"command"				"ReturnToLobby"
	}
			
	"BtnRestartScenario"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnRestartScenario"
		"xpos"					"0"
		"ypos"					"20"
		"wide"					"150"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnReturnToLobby"
		"navDown"				"BtnChangeChapter"
		"tooltiptext"			"#L4D360UI_RestartChapter_Tip"
		"labelText"				"#L4D360UI_RestartChapter"
		"style"					"FlyoutMenuButton"
		"command"				"RestartScenario"
	}
	
	"BtnChangeChapter"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnChangeChapter"
		"xpos"					"0"
		"ypos"					"40"
		"wide"					"150"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnRestartScenario"
		"navDown"				"BtnBootPlayer"
		"tooltiptext"			"#L4D360UI_ChangeChapter_Tip"
		"labelText"				"#L4D360UI_ChangeChapter"
		"style"					"FlyoutMenuButton"
		"command"				"ChangeChapter"
	}
	
	"BtnBootPlayer"
	{
		"ControlName"			"BaseModHybridButton"
		"fieldName"				"BtnBootPlayer"
		"xpos"					"0"
		"ypos"					"60"
		"wide"					"150"
		"tall"					"20"
		"autoResize"			"1"
		"pinCorner"				"0"
		"visible"				"1"
		"enabled"				"1"
		"tabPosition"			"0"
		"navUp"					"BtnChangeChapter"
		"navDown"				"BtnReturnToLobby"
		"tooltiptext"			"#L4D360UI_BootPlayer_Tip"
		"labelText"				"#L4D360UI_BootPlayer"
		"style"					"FlyoutMenuButton"
		"command"				"BootPlayer"
	}
}