**********
Contact us
**********

.. raw:: html

    <style type="text/css">
    form.support {
        margin: 1.5em 0em;
        padding: 1.5em 0em;
        border-top: 1px solid rgba(0,0,0,.1);
    }
    form.support input {
        display: block;
        margin: 1em 0em;
    }
    </style>
    
    <form class="support">
        <label>Name</label>
        <input id="nameField" type="text" name="name" size="50" maxlength="30" placeholder="Your name" required>
        <label>Email</label>
        <input id="emailField" type="email" name="email" size="50" maxlength="30" placeholder="you@yourcompany.com" required>
        <label>Message</label>
        <textarea id="messageField" name="message" cols="50" rows="10" maxlenght="500" placeholder="Enter your message" spellcheck="true" style="width: 55%" required></textarea>

        <input id="submitButton" type="submit" value="Send" class="btn">
        <input id="titleField" type="hidden" name="title" value="Request Support">
        <input id="tokenField" type="hidden" value="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"/>
    </form>

    <script>
        $('#submitButton').on('click',function(){
        $.ajax({
            url: "https://api.github.com/orgs/WizeEveryWhere/teams/support/discussions",
            type: 'POST',
            data: JSON.stringify( {
                    "title": $('#titleField').val() + " from : " + $('#nameField').val() + " (" + $('#emailField').val() + ")",
                    "body": $('#messageField').val(),
                    "private": true 
                    } ),
            contentType: 'application/json',
            headers: {
                        "Accept": "application/vnd.github+json",
                        "Authorization": "Bearer " + $('#tokenField').val()
                    },
            async: false
                })
        });
    </script>
