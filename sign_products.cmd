IF "%1" EQU "true" (
	REM Sign products

	call sign_modern_context_menu_products.cmd --path "build-x64-windows-mega\src\MEGAShellExt\RelWithDebInfo" || exit 1 /b

	IF "%MEGA_SKIP_32_BIT_BUILD%" == "true" (
		GOTO :EOF
	)

	call sign_modern_context_menu_products.cmd --path "build-x86-windows-mega\src\MEGAShellExt\RelWithDebInfo" || exit 1 /b
) ELSE (
	echo "Sign products skipped."
)

