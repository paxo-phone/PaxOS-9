function run()
    time:setTimeout(function()
        launch("testbc")
        system.app:quit()
    end, 1000)
end
