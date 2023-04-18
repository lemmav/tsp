$(document).ready(function () {
    
    function createTable(size) {
        $("#graph-table").empty();
        for (let y = 0; y < size; y++) {
            let row = document.createElement("tr")
            for (let x = 0; x < size; x++) {
                //let cell = $("<td/>")
                //let elem = $("<input/>", {
                //    pattern: "\\d*(\\.?d*)"
                //});
                //elem.addClass("graph-input");
                //elem.appendTo(cell);
                $(row).append($('<td><input class="graph-input" pattern="\\d*(\\.?\\d*)"></td>'))
            }
            $("#graph-table").append(row);
        }
    }

    createTable(5);

    $("#size").change(function (e) { 
        e.preventDefault();
        createTable($("#size").val());
    });

    $("#submit-btn").click(function (e) { 
        e.preventDefault();
        let data = $("#graph-form-main").serializeArray()

        let graph_string = $("#size").val() + "\r\n"
        $("#graph-table").children().each(function (_, row) { 
            let arr = []
            console.log(row)
            $(row).children().each(function (_, cell) { 
                let val = $(cell).children("input").val();
                if (val === "") val = "0"
                arr.push(val);
            });
            console.log(arr)
            graph_string += arr.join(" ") + "\r\n";
        });
        console.log(data);
        data.push({
            "name": "graph",
            "value": graph_string
        });
        console.log(data);
        
        $.ajax({
            type: "post",
            url: main_url,
            data: data,
            success: function (data, textStatus) {
                if (data.redirect) {
                    window.location.href = data.redirect;
                }
            }
        });
    });
});