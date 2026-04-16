\ = [System.IO.File]::ReadAllText('qwt_plot_renderer.h', [System.Text.Encoding]::UTF8)

# Constructor
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Constructor[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Constructor'

# Destructor
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Destructor[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Destructor'

# Set discard flag
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Set a discard flag[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Set a discard flag'

# Test discard flag
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Test a discard flag[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Test a discard flag'

# Set discard flags
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Set discard flags[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Set discard flags'

# Get discard flags
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Get discard flags[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Get discard flags'

# Set layout flag
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Set a layout flag[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Set a layout flag'

# Test layout flag
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Test a layout flag[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Test a layout flag'

# Set layout flags
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Set layout flags[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Set layout flags'

# Get layout flags
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Get layout flags[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Get layout flags'

# Render to document
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot to a document[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot to a document'

# Render to document with format
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot to a document with specified format[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot to a document with specified format'

# Render to SVG
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot to an SVG generator[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot to an SVG generator'

# Render to printer
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot to a printer[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot to a printer'

# Render to paint device
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot to a paint device[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot to a paint device'

# Render the plot
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the plot[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the plot'

# Render title
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the title[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the title'

# Render footer
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the footer[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the footer'

# Render scale
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render a scale[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render a scale'

# Render canvas
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the canvas[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the canvas'

# Render legend
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Render the legend[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Render the legend'

# Export to document
\ = \ -replace '    /\*\*[\r\n]+     \* \\if ENGLISH[\r\n]+     \* @brief Export the plot to a document[\r\n]+     \* \\endif[\r\n]+     \*/', '    // Export the plot to a document'

[System.IO.File]::WriteAllText('qwt_plot_renderer.h', \, [System.Text.Encoding]::UTF8)